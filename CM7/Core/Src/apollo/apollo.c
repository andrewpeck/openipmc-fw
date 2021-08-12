#include "apollo.h"
#include "stm32h7xx_hal.h"
#include "ipmc_ios.h"
#include "../dimm_gpios.h"
#include "cmsis_os.h"
#include "user_eeprom.h"
#include "sensor_helper.h"
#include "apollo_i2c.h"
#include "cm_sensors.h"
#include "sm_sensors.h"
#include "pim400.h"
#include "stdint.h"
#include "zynq_i2c.h"

uint8_t apollo_abormal_shutdown = 0;
uint8_t apollo_startup_started  = 0;
uint8_t apollo_startup_done     = 0;
uint8_t apollo_boot_mode        = APOLLO_BOOT_SD;

uint8_t apollo_status       = 0;

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

  const uint16_t max = seconds * (1000 / interval_ms);

  for (uint16_t i = 0; i < max; i++) {

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

  // timeout, shutdown!
  apollo_startup_started=0;
  apollo_abormal_shutdown=err;
  apollo_powerdown_sequence();
  return 1;

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

void apollo_powerdown_sequence() {

  apollo_startup_done = 0;
  apollo_startup_started = 0;

  ipmc_ios_printf("Powering Down Service Module:\r\n");

  // ask zynq to shut down
  ipmc_ios_printf(" > Requesting zynq shutdown...\r\n");
  zynq_request_shutdown();

  // disable zynq
  ipmc_ios_printf(" > Disabling Zynq...\r\n");
  apollo_set_zynq_en(0);

  // wait for zynq to shut down
  ipmc_ios_printf(" > Waiting for Zynq to shut down...\r\n");
  apollo_status = APOLLO_STATUS_PD_WAIT_ZYNQ_OFF;

  const uint8_t seconds = 20;

  for (int8_t i = 0; i < seconds * 10; i++) {
    if (apollo_get_zynq_done_generic() == 0) {
      break;
    }
    osDelay(100);
  }

  if (apollo_get_zynq_done_generic() == 1) {
    apollo_abormal_shutdown = APOLLO_ERR_TIMEOUT_ZYNQ_SHUTDOWN;
  }

  apollo_status = APOLLO_STATUS_PD_TIMEOUT_ZYNQ_OFF;

  // turn off power
  apollo_status = APOLLO_STATUS_PD_12V_PD;
  ipmc_ios_printf(" > Disabling 12V Power...\r\n");
  EN_12V_SET_STATE(RESET);
  apollo_status = APOLLO_STATUS_PD_DONE;
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

  ipmc_ios_printf("Powering Up Service Module:\r\n");

  // reset i2c sense mux
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_RESET_I2C_MUX;
  ipmc_ios_printf(" > Resetting MGM I2C Mux...\r\n");
  apollo_sense_reset ();

  // Read from EEPROM
  //------------------------------------------------------------------------------

  apollo_status = APOLLO_STATUS_READ_EEPROM;
  user_eeprom_init();
  if (0==user_eeprom_read()) {
   // set apollo_boot_mode to the retval of this
   user_eeprom_get_boot_mode(&apollo_boot_mode);
  }

  // set boot pins
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_BOOT_MODE;
  ipmc_ios_printf(" > Setting boot mode to 0x%1X...\r\n", apollo_boot_mode);
  apollo_set_zynq_boot_mode (apollo_boot_mode);

  // set uart pins
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_UART_ADR;
  uint8_t uart_adr = APOLLO_UART_DISCONNECT;
  ipmc_ios_printf(" > Setting uart adr to 0x%1X...\r\n", uart_adr);
  apollo_set_uart_adr (uart_adr);

  // set jtag chain select pins
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_CHAIN_SEL;
  uint8_t chain_sel=0x0;
  if (revision == APOLLO_REV1) {
    // chain sel has a different meaning in rev0
    chain_sel = 0;
  }
  ipmc_ios_printf(" > Setting jtag chain sel to 0x%1X...\r\n", chain_sel);
  apollo_set_jtag_chain_sel (chain_sel);

  // turn off we on eeprom
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_EEPROM_WE;
  apollo_set_eeprom_we_n (1);

  osDelay(10);

  // turn on power
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_12V_EN;
  ipmc_ios_printf(" > Enabling 12V power...\r\n");
  EN_12V_SET_STATE(SET);

  osDelay(100);

  // set zynq enable
  //------------------------------------------------------------------------------
  apollo_status = APOLLO_STATUS_PU_SET_ZYNQ_EN;
  ipmc_ios_printf(" > Enabling Zynq...\r\n");
  apollo_set_zynq_en(1);
  ipmc_ios_printf(" > Zynq Enabled...\r\n");

  // for SMv1
  //------------------------------------------------------------------------------

  //if (revision == APOLLO_REV1) {
  //    ipmc_ios_printf(" > Waiting for ESM Power Good...\r\n");
  //    while (0==apollo_get_esm_pwr_good()) {;}
  //    ipmc_ios_printf(" > ESM Power Good...\r\n");
  //    LED_0_SET_STATE(SET);
  // TODO: timeout
  // turn off power  ?
  //}

  // for SMv2 wait for the ZYNQ FPGA to configure
  //------------------------------------------------------------------------------
  if (revision == APOLLO_REV2 || revision == APOLLO_REV2A) {

    apollo_status = APOLLO_STATUS_PU_WAIT_FPGA_DONE;

    ipmc_ios_printf(" > Waiting for Zynq FPGA...\r\n");

    // wait for fpga to go up, if it doesn't then shut back down
    if (apollo_timeout_counter (apollo_get_fpga_done, 90, 100, APOLLO_ERR_TIMEOUT_ZYNQ_FPGA)) {
      apollo_status = APOLLO_STATUS_PU_TIMEOUT_FPGA_DONE;
      return;
    }

    ipmc_ios_printf(" > ZYNQ FPGA DONE...\r\n");
    LED_0_SET_STATE(SET);

    // TODO: timeout
    // turn off power  ?
  } else if (revision == APOLLO_REV1) {
    osDelay(300);
  }

  // set uart pins
  //------------------------------------------------------------------------------

  uart_adr = APOLLO_UART_ZYNQ;
  ipmc_ios_printf(" > Setting uart adr to 0x%1X...\r\n", uart_adr);
  // FIXME: for now we just set to zero
  uart_adr = 0; // 0 == connect to Zynq
  apollo_set_uart_adr (uart_adr);

  ipmc_ios_printf(" > Powerup done\n", uart_adr);

  // zynq timeout
  //------------------------------------------------------------------------------

  ipmc_ios_printf(" > Waiting for Zynq OS...\r\n");

  // Zynq up is from Linux
  ipmc_ios_printf(" > SMRev2: Waiting for Zynq Up..\r\n");

  apollo_status = APOLLO_STATUS_PU_WAIT_ZYNQ_DONE;
    // wait for zynq to go up, if it doesn't then shut back down
  if (apollo_timeout_counter(apollo_get_zynq_done_generic, 90, 100, APOLLO_ERR_TIMEOUT_ZYNQ_CPU)) {
    apollo_status = APOLLO_STATUS_PU_TIMEOUT_ZYNQ_DONE;
    return;
  }

  // if (revision==APOLLO_REV1) {
  //   apollo_esm_reset(100);
  // }

  // write zynq constants
  //------------------------------------------------------------------------------

  apollo_write_zynq_i2c_constants();

  // Fini
  //------------------------------------------------------------------------------

  apollo_status = APOLLO_STATUS_PU_DONE;

  apollo_startup_done = 1;

}

void apollo_write_zynq_i2c_constants () {

  if (apollo_get_zynq_done_generic ()) {
      // sn
      uint8_t sn;
      user_eeprom_get_serial_number(&sn);
      zynq_set_blade_sn(sn);

      // slot
      zynq_set_blade_slot(ipmc_ios_read_haddress()); // FIXME: divide by 2, multiply by 2???

      // revision
      zynq_set_blade_rev(apollo_get_revision());

      // FIXME, somehow retrieve the ip and mac from the ethernet_if instead of
      // re-calculating it here

      // ip
      uint8_t ip [4] = {192,168,21,ipmc_ios_read_haddress()};
      zynq_set_ipmc_ip(ip);

      // mac
      uint32_t id = HAL_GetUIDw0() + HAL_GetUIDw1() + HAL_GetUIDw2();
      uint8_t mac [6] = {0x00, 0x80, 0xe1, (id >> 16)&0xFF, (id >> 8)&0xFF, (id >> 0)&0xFF};
      zynq_set_ipmc_mac(mac);
    }
}

void board_specific_activation_control(uint8_t current_power_level,
                                       uint8_t new_power_level) {
  if (new_power_level == 0) {
    apollo_powerdown_sequence();
  } else {
    apollo_powerup_sequence();
  }
  return;
}

void payload_cold_reset(void) {
  PAYLOAD_RESET_SET_STATE(RESET);
  apollo_powerdown_sequence();

  PAYLOAD_RESET_SET_STATE(SET);
  apollo_powerup_sequence();
}

void board_specific_sensor_inits() {

  //------------------------------------------------------------------------------
  // SM
  //------------------------------------------------------------------------------

  create_linear_sensor (sm_tcn_temp_consts, "SM Top Temperature", &sensor_reading_sm_tcn_top);
  create_linear_sensor (sm_tcn_temp_consts, "SM Mid Temperature", &sensor_reading_sm_tcn_mid);
  create_linear_sensor (sm_tcn_temp_consts, "SM Bot Temperature", &sensor_reading_sm_tcn_bot);

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

}
