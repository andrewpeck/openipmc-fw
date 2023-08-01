#include "apollo.h"
#include "printf.h"
#include "stm32h7xx_hal.h"
#include "ipmc_ios.h"
#include "dimm_gpios.h"
#include "cmsis_os.h"
#include "user_eeprom.h"
#include "sensor_helper.h"
#include "apollo_i2c.h"
#include "cm_sensors.h"
#include "sm_sensors.h"
#include "ip_sensors.h"
#include "picmg_address_info.h"
#include "pim400.h"
#include "stdint.h"
#include "zynq_i2c.h"
#include "zynq_temp_sensor.h"
#include "head_commit_sha1.h"
#include "apollo_i2c_mutex.h"

#include "lwip/netif.h"
#include "lwip/ip4_addr.h"

extern uint8_t shelf_address;
extern uint8_t shelf_address_type;
void mt_printf(const char* format, ...);

uint8_t apollo_abormal_shutdown = 0;
uint8_t apollo_startup_started  = 0;
uint8_t apollo_startup_done     = 0;
uint8_t apollo_boot_mode        = APOLLO_BOOT_SD;
uint8_t apollo_sdsel            = APOLLO_SDSEL_MIDBOARD;

uint8_t apollo_status       = 0;

/* Mutex for the I2C transactions using the I2C3 bus. */
uint8_t apollo_i2c3_mutex = APOLLO_I2C3_MUTEX_UNLOCKED;

uint8_t apollo_get_handle_open () {
  if (ipmc_ios_read_handle() == APOLLO_HANDLE_OPEN) {
    return 1;
  }
  return 0;
}

uint8_t apollo_timeout_counter(uint8_t (*check_function)(),
    const uint8_t seconds,
    const uint16_t interval_ms,
    const uint8_t err) {

  uint8_t dis_shutoff=1;
  user_eeprom_get_disable_shutoff(&dis_shutoff);
  if (dis_shutoff) {
    return 0;
  }
  else {
    const uint16_t max = seconds * (1000 / interval_ms);

    for (uint16_t i = 0; i < max; i++) {

      mt_printf("     ...\r\n");

      // just poll periodically, to allow the os to do other things
      osDelay(interval_ms);

      // returned successfully
      if ((*check_function)() == 1) {
        return 0;
      }

      // somebody opened the handle
      if (apollo_get_handle_open()) {
        apollo_powerdown_sequence();
        apollo_startup_started = 0;
        apollo_abormal_shutdown=APOLLO_ERR_OPEN_HANDLE;
        return 1;
      }
    }

    // timeout, shutdown! (if dis_shutoff is 0)
    apollo_startup_started=0;
    apollo_abormal_shutdown=err;
    apollo_powerdown_sequence();
    return 1;
  }
}

void apollo_init_gpios () {
  // choices from stm32f4xx__hal__gpio_8h_source.html
  // GPIO_MODE_INPUT                        /*!< Input Floating Mode                   */
  // GPIO_MODE_OUTPUT_PP                    /*!< Output Push Pull Mode                 */
  // GPIO_MODE_OUTPUT_OD                    /*!< Output Open Drain Mode                */
  // GPIO_MODE_AF_PP                        /*!< Alternate Function Push Pull Mode     */
  // GPIO_MODE_AF_OD                        /*!< Alternate Function Open Drain Mode    */
  // GPIO_MODE_ANALOG                       /*!< Analog Mode  */
  // GPIO_MODE_IT_RISING                    /*!< External Interrupt Mode with Rising edge trigger detection          */
  // GPIO_MODE_IT_FALLING                   /*!< External Interrupt Mode with Falling edge trigger detection         */
  // GPIO_MODE_IT_RISING_FALLING            /*!< External Interrupt Mode with Rising/Falling edge trigger detection  */
  // GPIO_MODE_EVT_RISING                   /*!< External Event Mode with Rising edge trigger detection               */
  // GPIO_MODE_EVT_FALLING                  /*!< External Event Mode with Falling edge trigger detection              */
  // GPIO_MODE_EVT_RISING_FALLING           /*!< External Event Mode with Rising/Falling edge trigger detection       */
  //
  // GPIO_NOPULL        /*!< No Pull-up or Pull-down activation  */
  // GPIO_PULLUP        /*!< 100k Pull-up activation                  */
  // GPIO_PULLDOWN      /*!< 100k Pull-down activation                */

  // outputs
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_UART_ADR_0       , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_UART_ADR_1       , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
  GPIO_SET_STATE_EXPAND (1, APOLLO_UART_ADR_0); // default
  GPIO_SET_STATE_EXPAND (0, APOLLO_UART_ADR_1); // default

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_BOOT_MODE_0      , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_BOOT_MODE_1      , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
  GPIO_SET_STATE_EXPAND (1, APOLLO_BOOT_MODE_0); // default
  GPIO_SET_STATE_EXPAND (1, APOLLO_BOOT_MODE_1); // default

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_SENSE_RESET_N    , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL); // pullup, 4.7k
  GPIO_SET_STATE_EXPAND (1, APOLLO_SENSE_RESET_N); // default

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_MEZZ1_EN         , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL); // default control via zynq; resistor jumper to choose
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_MEZZ2_EN         , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL); // default control via zynq; resistor jumper to choose
  GPIO_SET_STATE_EXPAND (0, APOLLO_MEZZ1_EN); // default
  GPIO_SET_STATE_EXPAND (0, APOLLO_MEZZ2_EN); // default

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_M24512_WE_N      , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
  GPIO_SET_STATE_EXPAND (1, APOLLO_M24512_WE_N); // default

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_ZYNQ_EN          , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL); // pulled down 1.1k
  GPIO_SET_STATE_EXPAND (0, APOLLO_ZYNQ_EN); // default

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_ESM_RESET_N      , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
  GPIO_SET_STATE_EXPAND (1, APOLLO_ESM_RESET_N); // default

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_JTAG_CHAIN_SEL_0 , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_JTAG_CHAIN_SEL_1 , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_JTAG_CHAIN_SEL_2 , GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
  GPIO_SET_STATE_EXPAND (0, APOLLO_JTAG_CHAIN_SEL_0); // default
  GPIO_SET_STATE_EXPAND (0, APOLLO_JTAG_CHAIN_SEL_1); // default
  GPIO_SET_STATE_EXPAND (0, APOLLO_JTAG_CHAIN_SEL_2); // default

  // inputs; pull-ups = 100k
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_ETH_SW_PWR_GOOD  , GPIO_MODE_INPUT, GPIO_PULLDOWN); //

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_ZYNQ_UP          , GPIO_MODE_INPUT, GPIO_PULLDOWN); // pulled down 4.6k

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_FPGA_DONE        , GPIO_MODE_INPUT, GPIO_PULLDOWN);

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_NO_SHELF         , GPIO_MODE_INPUT, GPIO_PULLUP);

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_REVID_0          , GPIO_MODE_INPUT, GPIO_PULLUP);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_REVID_1          , GPIO_MODE_INPUT, GPIO_PULLUP);

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_GPIO_0           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_GPIO_1           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_GPIO_2           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_GPIO_3           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_GPIO_4           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_GPIO_5           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_GPIO_6           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_GPIO_7           , GPIO_MODE_INPUT, GPIO_PULLDOWN);

  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_ZYNQ_0           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_ZYNQ_1           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_ZYNQ_2           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_ZYNQ_3           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_ZYNQ_4           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
  GPIO_CONFIGURE_PIN_EXPAND (APOLLO_ZYNQ_5           , GPIO_MODE_INPUT, GPIO_PULLDOWN);
}

uint8_t apollo_get_esm_pwr_good () {
  uint8_t state = GPIO_GET_STATE_EXPAND (APOLLO_ETH_SW_PWR_GOOD);
  return state;
}

uint8_t apollo_get_ipmc_startup_done () {
  return apollo_startup_done;
}

uint8_t apollo_get_ipmc_startup_started () {
  return apollo_startup_started;
}

uint8_t apollo_get_ipmc_abnormal_shutdown () {
  return apollo_abormal_shutdown;
}

uint8_t apollo_get_fpga_done () {
  uint8_t revision = apollo_get_revision();

  if (revision == APOLLO_REV1)
    // this pin does not exist in rev1
    return 1;
  else
    return GPIO_GET_STATE_EXPAND (APOLLO_FPGA_DONE);

}

uint8_t apollo_get_revision () {
  uint8_t state = GPIO_GET_STATE_EXPAND (APOLLO_REVID_0);
  state |= (GPIO_GET_STATE_EXPAND (APOLLO_REVID_1) << 1);
  return state;
}

void apollo_set_uart_adr (uint8_t adr) {
  GPIO_SET_STATE_EXPAND ((adr>>0) & 1, APOLLO_UART_ADR_0);
  GPIO_SET_STATE_EXPAND ((adr>>1) & 1, APOLLO_UART_ADR_1);
}

void apollo_set_sdsel (uint8_t sdsel) {
  GPIO_SET_STATE_EXPAND (sdsel, APOLLO_SDSEL);
}

uint8_t apollo_get_sdsel () {
  return GPIO_GET_STATE_EXPAND (APOLLO_SDSEL);
}

void apollo_set_zynq_boot_mode (uint8_t mode) {
  apollo_boot_mode = mode;
  GPIO_SET_STATE_EXPAND ((mode>>0) & 1, APOLLO_BOOT_MODE_0);
  GPIO_SET_STATE_EXPAND ((mode>>1) & 1, APOLLO_BOOT_MODE_1);
}

uint8_t apollo_get_zynq_boot_mode () {
  uint8_t mode=0;
  mode |= GPIO_GET_STATE_EXPAND (APOLLO_BOOT_MODE_0);
  mode |= GPIO_GET_STATE_EXPAND (APOLLO_BOOT_MODE_1) << 1;
  return mode;
}

void apollo_set_jtag_chain_sel (uint8_t sel) {
  GPIO_SET_STATE_EXPAND ((sel >> 0) & 1, APOLLO_JTAG_CHAIN_SEL_0 );
  GPIO_SET_STATE_EXPAND ((sel >> 1) & 1, APOLLO_JTAG_CHAIN_SEL_1 );
  GPIO_SET_STATE_EXPAND ((sel >> 2) & 1, APOLLO_JTAG_CHAIN_SEL_2 );
}

void apollo_set_cm_enable (uint8_t enable) {
  GPIO_SET_STATE_EXPAND ((enable >> 0) & 1, APOLLO_MEZZ1_EN);
  GPIO_SET_STATE_EXPAND ((enable >> 1) & 1, APOLLO_MEZZ2_EN);
}

void apollo_set_sense_reset_n (uint8_t state) {
  GPIO_SET_STATE_EXPAND (state & 1, APOLLO_SENSE_RESET_N);
}

void apollo_set_eeprom_we_n (uint8_t state) {
  GPIO_SET_STATE_EXPAND (state & 1, APOLLO_M24512_WE_N);
}

void apollo_set_esm_reset_n (uint8_t state) {
  GPIO_SET_STATE_EXPAND (state & 1, APOLLO_ESM_RESET_N);
}

void apollo_set_zynq_en (uint8_t state) {
  GPIO_SET_STATE_EXPAND (state & 1, APOLLO_ZYNQ_EN);
}

uint8_t apollo_get_zynq_en () {
  return GPIO_GET_STATE_EXPAND (APOLLO_ZYNQ_EN);
}

uint8_t apollo_get_zynq_up () {
  uint8_t state = GPIO_GET_STATE_EXPAND (APOLLO_ZYNQ_UP);
  return state;
}

uint8_t apollo_get_noshelf () {
  uint8_t state = GPIO_GET_STATE_EXPAND (APOLLO_NO_SHELF);
  return state;
}

void apollo_sense_reset() {
  apollo_set_sense_reset_n(0);
  osDelay(10); // Holds for 10ms
  apollo_set_sense_reset_n(1);
}

void apollo_esm_reset(const int delay) {
  apollo_set_esm_reset_n(0);
  osDelay(delay); // Holds for delay ms
  apollo_set_esm_reset_n(1);
}

uint8_t apollo_get_zynq_done_generic () {
  uint8_t revision = apollo_get_revision();

  if (revision == APOLLO_REV1)
    return zynq_get_i2c_done();
  else
    return apollo_get_zynq_up();
}

void apollo_set_site_number() {
  /*
   * Send the site number obtained from shelf manager to the S6 I2C target on the Zynq.
   * If the site number retrieval fails, prints a message to the console.
   */
  picmg_address_info_data_t fru_addr_data;
  int addr_status;
  addr_status = picmg_get_address_info(&fru_addr_data);
  if (addr_status == PICMG_ADDRESS_INFO_ERR_OK) {
    zynq_set_site_number(fru_addr_data.site_number);
  }
  // Failed to retrieve site number from the shelf manager
  else {
    mt_printf(" > Failed to retrieve FRU address data.\r\n");
  }
}

void apollo_set_shelf_id() {
  /*
   * Send the shelf ID data (20 bytes) to the S6 I2C target on the Zynq.
   * The unused bytes are set to 0x00.
   * If the shelf ID retrieval fails, prints a message to the console.
   */
  picmg_shelf_address_info_data_t shelf_addr_data;
  int addr_status;
  addr_status = picmg_get_shelf_address_info(&shelf_addr_data);
  /*
   * Shelf ID data is 20 bytes maximum. Check the length of data (i.e. number of bytes)
   * and assign NULL value (0x00) to unused bytes. 
   */
  if (addr_status == PICMG_GET_SHELF_ADDRESS_ERR_OK) {
    uint8_t shelf_id_num_bytes = 20;
    uint8_t shelf_id_data[shelf_id_num_bytes];

    // Number of actually used bytes
    uint8_t shelf_id_num_used_bytes = shelf_addr_data.len;

    for (uint8_t i=0; i < shelf_id_num_used_bytes; i++) {
      shelf_id_data[i] = shelf_addr_data.data[i];
    }

    // Pad the remaining data with NULL (0x00)
    for (uint8_t i=shelf_id_num_used_bytes; i < shelf_id_num_bytes; i++) {
      shelf_id_data[i] = 0x00;
    }
    zynq_set_shelf_id(shelf_id_data);
  }
  // Failed to retrieve shelf ID from the shelf manager
  else {
    mt_printf(" > Failed to retrieve shelf ID data.\r\n");
  }
}

void apollo_powerdown_sequence() {

  apollo_startup_done = 0;
  apollo_startup_started = 0;

  mt_printf("Powering Down Service Module:\r\n");

  // ask zynq (nicely) to shut down
  mt_printf(" > Requesting zynq shutdown\r\n");
  zynq_request_shutdown();

  // wait for zynq to shut down
  mt_printf(" > Waiting for Zynq to shut down\r\n");
  apollo_status = APOLLO_STATUS_PD_WAIT_ZYNQ_OFF;

  const uint8_t seconds = 10;

  for (int8_t i = 0; i < seconds * 10; i++) {
    if (apollo_get_zynq_done_generic() == 0) {
      break;
    }
    osDelay(100);
    if (i % 10 == 0) {
      mt_printf("     ...\r\n");
    }
  }

  if (apollo_get_zynq_done_generic() == 1) {
    apollo_abormal_shutdown = APOLLO_ERR_TIMEOUT_ZYNQ_SHUTDOWN;
    mt_printf("   Zynq failed to shut down properly!! powering off anyway\r\n");
  } else {
    mt_printf("   Zynq shut down ok.. powering off\r\n");
  }

  // disable zynq
  mt_printf(" > Disabling Zynq Enable Line\r\n");
  apollo_set_zynq_en(0);

  osDelay(500);

  apollo_status = APOLLO_STATUS_PD_TIMEOUT_ZYNQ_OFF;

  // turn off power
  apollo_status = APOLLO_STATUS_PD_12V_PD;

  mt_printf(" > Disabling 12V Power\r\n");

  EN_12V_SET_STATE(RESET);
  mt_printf("   12V power disabled\r\n");
  apollo_status = APOLLO_STATUS_PD_DONE;
  mt_printf(" > Shutdown done\r\n");
  return;
}

void apollo_init_bootmode () {

  uint8_t boot_mode = APOLLO_BOOT_SD;

  const uint8_t revision=apollo_get_revision();
  if (revision == APOLLO_REV2 || revision == APOLLO_REV2A) {
    if (apollo_get_noshelf()) {
      boot_mode = APOLLO_BOOT_JTAG; // JTAG / EMMC
    } else {
      boot_mode = APOLLO_BOOT_SD;
    }
  }

  apollo_set_zynq_boot_mode (boot_mode);

}

char* get_apollo_status () {

  if (apollo_get_handle_open()) {
      return "Front Panel Handle Open";
  }

  if (apollo_abormal_shutdown) {
    switch (apollo_abormal_shutdown) {
      case APOLLO_ERR_TIMEOUT_ZYNQ_FPGA:
        return "ERR: Powerup Timed out waiting for Zynq FPGA";
        break; 
      case APOLLO_ERR_TIMEOUT_ZYNQ_CPU:
        return "ERR: Powerup Timed out waiting for Zynq CPU";
              break; 
      case APOLLO_ERR_TIMEOUT_ZYNQ_SHUTDOWN:
        return "ERR: Powerdown Timed out waiting for Zynq to turn off";
              break; 
      case APOLLO_ERR_OPEN_HANDLE:
        return "ERR: Handle opened during power sequence";
              break; 
      }
  }

  switch (apollo_status) {
    case APOLLO_STATUS_PU_INIT_IO:
      return "Powerup: Initializing IOs";
      break;
    case APOLLO_STATUS_PU_SET_BOOT_MODE:
      return "Powerup: Setting Boot Mode";
      break;
    case APOLLO_STATUS_PU_SET_UART_ADR:
      return "Powerup: Setting UART Address";
      break;
    case APOLLO_STATUS_PU_SET_CHAIN_SEL:
      return "Powerup: Setting Chain Select";
      break;
    case APOLLO_STATUS_PU_SET_EEPROM_WE:
      return "Powerup: Setting EEPROM WE";
      break;
    case APOLLO_STATUS_PU_RESET_I2C_MUX:
      return "Powerup: Resetting I2C Mux";
      break;
    case APOLLO_STATUS_PU_SET_12V_EN:
      return "Powerup: Turning on 12V";
      break;
    case APOLLO_STATUS_PU_SET_ZYNQ_EN:
      return "Powerup: Setting Zynq Enable";
      break;
    case APOLLO_STATUS_PU_TIMEOUT_FPGA_DONE:
      return "Powerup: Timeout waiting for FPGA DONE";
      break;
    case APOLLO_STATUS_PU_WAIT_FPGA_DONE:
      return "Powerup: Waiting for FPGA DONE";
      break;
    case APOLLO_STATUS_PU_TIMEOUT_ZYNQ_DONE:
      return "Powerup: Timeout waiting for Zynq DONE";
      break;
    case APOLLO_STATUS_PU_WAIT_ZYNQ_DONE:
      return "Powerup: Waiting for Zynq DONE";
      break;
    case APOLLO_STATUS_PU_DONE:
      return "Powerup: Powerup Finished";
      break;
    case APOLLO_STATUS_PD_WAIT_ZYNQ_OFF:
      return "Powerdown: Waiting for Zynq to turn off";
      break;
    case APOLLO_STATUS_PD_TIMEOUT_ZYNQ_OFF:
      return "Powerdown: Timeout waiting for zynq to turn off";
      break;
    case APOLLO_STATUS_PD_12V_PD:
      return "Powerdown: Turning off 12V power";
      break;
    case APOLLO_STATUS_PD_DONE:
      return "Powerdown: Powerdown Finished";
      break;
    case APOLLO_STATUS_READ_EEPROM:
      return "Powerup: Reading EEPROM";
      break;
    default:
      return "Unknown state.. oh no";
      break;
  }
}

void apollo_powerup_sequence () {

  apollo_startup_started=1;
  apollo_abormal_shutdown=0;

  // re-init to their default states
  apollo_status = APOLLO_STATUS_PU_INIT_IO;
  apollo_init_gpios ();


  osDelay(100);

  const uint8_t revision=apollo_get_revision();

  LED_0_SET_STATE(RESET);
  LED_1_SET_STATE(RESET);

  // make sure everything is off
  //------------------------------------------------------------------------------
  EN_12V_SET_STATE(RESET);
  apollo_set_zynq_en(0);

  mt_printf("Powering Up Service Module:\r\n");

  // reset i2c sense mux
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_RESET_I2C_MUX;
  mt_printf(" > Resetting MGM I2C Mux\r\n");
  apollo_sense_reset ();

  // Read from EEPROM
  //------------------------------------------------------------------------------

  apollo_status = APOLLO_STATUS_READ_EEPROM;
  user_eeprom_init();
  if (0==user_eeprom_read()) {
   // set apollo_boot_mode to the retval of this
   user_eeprom_get_boot_mode(&apollo_boot_mode);
   user_eeprom_get_sdsel(&apollo_sdsel);
  }

  // set sd card select
  //------------------------------------------------------------------------------
  apollo_set_sdsel(apollo_sdsel);

  // set boot pins
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_BOOT_MODE;
  mt_printf(" > Setting boot mode to 0x%1X\r\n", apollo_boot_mode);
  apollo_set_zynq_boot_mode (apollo_boot_mode);

  // set uart pins
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_UART_ADR;
  uint8_t uart_adr = APOLLO_UART_DISCONNECT;
  mt_printf(" > Setting uart adr to 0x%1X\r\n", uart_adr);
  apollo_set_uart_adr (uart_adr);

  // set jtag chain select pins
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_CHAIN_SEL;
  uint8_t chain_sel=0x0;
  if (revision == APOLLO_REV1) {
    // chain sel has a different meaning in rev0
    chain_sel = 0;
  }
  mt_printf(" > Setting jtag chain sel to 0x%1X\r\n", chain_sel);
  apollo_set_jtag_chain_sel (chain_sel);

  // turn off we on eeprom
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_EEPROM_WE;
  apollo_set_eeprom_we_n (1);

  osDelay(10);

  // turn on power
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_12V_EN;
  mt_printf(" > Enabling 12V power\r\n");
  EN_12V_SET_STATE(SET);
  // set zynq enable
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_ZYNQ_EN;
  mt_printf(" > Enabling Zynq\r\n");
  apollo_set_zynq_en(1);
  mt_printf("   Zynq Enabled\r\n");

  // reset esm
  //------------------------------------------------------------------------------

  if (apollo_get_revision() == APOLLO_REV1) {

    mt_printf(" > Resetting ESM (rev1) \r\n");
    while (0==apollo_get_esm_pwr_good()) {}
    osDelay(100);

    apollo_esm_reset(25);
  }

  // for SMv1
  //------------------------------------------------------------------------------

  //if (revision == APOLLO_REV1) {
  //    mt_printf(" > Waiting for ESM Power Good\r\n");
  //    while (0==apollo_get_esm_pwr_good()) {;}
  //    mt_printf(" > ESM Power Good\r\n");
  //    LED_0_SET_STATE(SET);
  // TODO: timeout
  // turn off power  ?
  //}

  // for SMv2 wait for the ZYNQ FPGA to configure
  //------------------------------------------------------------------------------
  if (revision == APOLLO_REV2 || revision == APOLLO_REV2A) {

    apollo_status = APOLLO_STATUS_PU_WAIT_FPGA_DONE;

    mt_printf(" > Waiting for Zynq FPGA\r\n");

    // wait for fpga to go up, if it doesn't then shut back down
    if (apollo_timeout_counter (apollo_get_fpga_done, 90, 100, APOLLO_ERR_TIMEOUT_ZYNQ_FPGA)) {
      apollo_status = APOLLO_STATUS_PU_TIMEOUT_FPGA_DONE;
      return;
    }

    mt_printf(" > ZYNQ FPGA DONE\r\n");
    LED_0_SET_STATE(SET);

    // TODO: timeout
    // turn off power  ?
  } else if (revision == APOLLO_REV1) {
    osDelay(300);
  }

  // set uart pins
  //------------------------------------------------------------------------------

  uart_adr = APOLLO_UART_ZYNQ;
  mt_printf(" > Setting uart adr to 0x%1X\r\n", uart_adr);
  // FIXME: for now we just set to zero
  uart_adr = 0; // 0 == connect to Zynq
  apollo_set_uart_adr (uart_adr);

  mt_printf(" > Powerup done\n", uart_adr);

  // zynq timeout
  //------------------------------------------------------------------------------

  // Zynq up is from Linux
  mt_printf(" > Waiting for Zynq Up\r\n");

  apollo_status = APOLLO_STATUS_PU_WAIT_ZYNQ_DONE;
    // wait for zynq to go up, if it doesn't then shut back down
  if (apollo_timeout_counter(apollo_get_zynq_done_generic, 90, 100, APOLLO_ERR_TIMEOUT_ZYNQ_CPU)) {
    apollo_status = APOLLO_STATUS_PU_TIMEOUT_ZYNQ_DONE;
    return;
  }

  // write zynq constants
  //------------------------------------------------------------------------------

  mt_printf(" > Writing I2C Constants to Zynq\r\n");
  apollo_write_zynq_i2c_constants();

  // Fini
  //------------------------------------------------------------------------------

  apollo_status = APOLLO_STATUS_PU_DONE;

  apollo_startup_done = 1;

  mt_printf(" > Startup DONE\r\n");

}

void apollo_write_zynq_i2c_constants () {

  if (apollo_get_zynq_done_generic ()) {
      // sn
      uint8_t reading;
      user_eeprom_get_serial_number(&reading);
      zynq_set_blade_sn(reading);

      // slot
      zynq_set_blade_slot(ipmc_ios_read_haddress() * 2);

      // revision
      zynq_set_blade_rev(apollo_get_revision());

      // FW hash of the IPMC
      // This is 32-bits, so split into 8-bit chunks.
      uint8_t ipmc_fw_hash[4];
      for (uint8_t i=0; i<4; i++) {
        ipmc_fw_hash[i] = (HEAD_COMMIT_SHA1 >> i*8) & 0xFF;
      }

      zynq_set_ipmc_fw_hash(ipmc_fw_hash);

      // IP address of the IPMC, being read from the LWIP interface
      extern struct netif gnetif;
      ip4_addr_t* ipaddr  = netif_ip4_addr   ( &gnetif );

      // Get the 4 individual bytes from the 32-bit IP address
      uint8_t ip[4];
      for (uint8_t i=0; i<4; i++) {
        ip[i] = (ipaddr->addr >> i*8) & 0xFF;
      }

      zynq_set_ipmc_ip(ip);

      // MAC address: Read it from EEProm for the ETH0 and ETH1 ports and set them in Zynq's I2C interface
      uint8_t eth0_mac[6];
      user_eeprom_get_mac_addr(0, eth0_mac);
      zynq_set_eth_mac(0, eth0_mac);

      uint8_t eth1_mac[6];
      user_eeprom_get_mac_addr(1, eth1_mac);
      zynq_set_eth_mac(1, eth1_mac);

      // Check-sums for the MAC addresses
      uint8_t eth0_mac_checksum;
      uint8_t eth1_mac_checksum;

      user_eeprom_get_mac_eth_checksum(0, &eth0_mac_checksum);
      user_eeprom_get_mac_eth_checksum(1, &eth1_mac_checksum);

      zynq_set_eth_checksum(0, eth0_mac_checksum);
      zynq_set_eth_checksum(1, eth1_mac_checksum);

      // All writes to S1 slave are complete
      zynq_set_s1_i2c_writes_done();

      // MAC address of the IPMC 
      uint32_t id = HAL_GetUIDw0() + HAL_GetUIDw1() + HAL_GetUIDw2();
      uint8_t mac [6] = {0x00, 0x80, 0xe1, (id >> 16)&0xFF, (id >> 8)&0xFF, (id >> 0)&0xFF};
      zynq_set_ipmc_mac(mac);

      // Send the 32-bit UID of the IPMC to Zynq
      uint8_t ipmc_uid[4];
      for (uint8_t i=0; i<4; i++) {
        ipmc_uid[i] = (id >> i*8) & 0xFF;
      }
      zynq_set_ipmc_uid(ipmc_uid);

      // Obtain and write shelf & slot ID
      apollo_set_site_number();
      apollo_set_shelf_id();

      // // pim
      read_status_pim400(&reading);
      zynq_wr_reg(0x00, reading, 0x66);

      read_vholdup_pim400(&reading);
      zynq_wr_reg(0x04, reading, 0x66);

      read_iout_pim400(&reading);
      zynq_wr_reg(0x05, reading, 0x66);

      read_voltage_pim400(&reading, 0);
      zynq_wr_reg(0x06, reading, 0x66);

      read_voltage_pim400(&reading, 1);
      zynq_wr_reg(0x07, reading, 0x66);

      read_temp_pim400(&reading);
      zynq_wr_reg(0x08, reading, 0x66);

      // tcn
      uint8_t data [2];
      read_sm_tcn_raw(TCN_TOP, data);
      zynq_wr_reg(0x14, data[1], 0x66);
      zynq_wr_reg(0x15, data[0], 0x66);

      read_sm_tcn_raw(TCN_MID, data);
      zynq_wr_reg(0x16, data[1], 0x66);
      zynq_wr_reg(0x17, data[0], 0x66);

      read_sm_tcn_raw(TCN_BOT, data);
      zynq_wr_reg(0x18, data[1], 0x66);
      zynq_wr_reg(0x19, data[0], 0x66);

      zynq_wr_reg(0x08, shelf_address, 0x60);
      zynq_wr_reg(0x09, shelf_address_type, 0x60);

    }
}

void board_specific_activation_policy (uint8_t current_power_level,
                                       uint8_t new_power_level) {
  if (new_power_level == 0) {
    apollo_powerdown_sequence();
  } else {
    apollo_powerup_sequence();
  }
  return;
}

/*
 * Payload Cold Reset
 *
 * This function is called by OpenIPMC when a Cold Reset command is received
 * from Shelf Manager
 */
void payload_cold_reset(void) {
  PAYLOAD_RESET_SET_STATE(RESET);
  apollo_powerdown_sequence();

  PAYLOAD_RESET_SET_STATE(SET);
  apollo_powerup_sequence();
}

void create_board_specific_sensors() {

  //------------------------------------------------------------------------------
  // SM
  //------------------------------------------------------------------------------

  create_linear_sensor (sm_tcn_temp_consts,  "SM Top Temperature",  &sensor_reading_sm_tcn_top);
  create_linear_sensor (sm_tcn_temp_consts,  "SM Mid Temperature",  &sensor_reading_sm_tcn_mid);
  create_linear_sensor (sm_tcn_temp_consts,  "SM Bot Temperature",  &sensor_reading_sm_tcn_bot);
  create_linear_sensor (sm_zynq_temp_consts, "SM Zynq Temperature", &sensor_reading_zynq_temp );

  //------------------------------------------------------------------------------
  // CM
  //------------------------------------------------------------------------------

  create_linear_sensor (cm_fpga_temp_consts,      "CM FPGA1 Temperature",         &sensor_reading_cm1_temp);
  create_linear_sensor (cm_fpga_temp_consts,      "CM FPGA2 Temperature",         &sensor_reading_cm2_temp);
  create_linear_sensor (cm_firefly_temp_consts,   "CM Firefly Max Temperature",   &sensor_reading_cm_firefly_temp);
  create_linear_sensor (cm_regulator_temp_consts, "CM Regulator Max Temperature", &sensor_reading_cm_regulator_temp);
  create_linear_sensor (cm_mcu_temp_consts,       "CM MCU Temperature",           &sensor_reading_cm_mcu_temp);

  //------------------------------------------------------------------------------
  // PIM400
  //------------------------------------------------------------------------------

  create_linear_sensor (pim400_temp_consts, "PIM400 Temperature", &sensor_reading_temp_pim400);
  create_linear_sensor (pim400_iout_consts, "PIM400 Current", &sensor_reading_iout_pim400);
  create_linear_sensor (pim400_voltage_consts, "PIM400 -48V_A", &sensor_reading_voltage_a_pim400);
  create_linear_sensor (pim400_voltage_consts, "PIM400 -48V_B", &sensor_reading_voltage_b_pim400);

  //------------------------------------------------------------------------------
  // IPMC IP address sensors
  //------------------------------------------------------------------------------
  create_linear_sensor (ipmc_ip_consts,  "IPMC IP Byte 0",  &sensor_reading_ipmc_ip_addr_byte0);
  create_linear_sensor (ipmc_ip_consts,  "IPMC IP Byte 1",  &sensor_reading_ipmc_ip_addr_byte1);
  create_linear_sensor (ipmc_ip_consts,  "IPMC IP Byte 2",  &sensor_reading_ipmc_ip_addr_byte2);
  create_linear_sensor (ipmc_ip_consts,  "IPMC IP Byte 3",  &sensor_reading_ipmc_ip_addr_byte3);

}
