#include "apollo.h"

#include "ipmc_ios.h"
#include "../dimm_gpios.h"
#include "cmsis_os.h"

#define SECOND 0x155555 // ~1 seconds timeout

uint8_t APOLLO_STARTUP_DONE = 0;

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
  return APOLLO_STARTUP_DONE;
}

uint8_t apollo_get_fpga_done () {
  uint8_t state = GPIO_GET_STATE_EXPAND (APOLLO_FPGA_DONE);
  return state;
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
  GPIO_SET_STATE_EXPAND ((mode>>0) & 1, APOLLO_BOOT_MODE_0);
  GPIO_SET_STATE_EXPAND ((mode>>1) & 1, APOLLO_BOOT_MODE_1);
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

void apollo_powerdown_sequence () {

  ipmc_ios_printf("Powering Down Service Module:\r\n");

  // disable zynq
  ipmc_ios_printf(" > Disabling Zynq...\r\n");
  apollo_set_zynq_en(0);

  // wait for zynq to shut down
  if (revision == APOLLO_REV2 || revision == APOLLO_REV2A) {
    ipmc_ios_printf(" > Waiting for Zynq to shut down...\r\n");
    uint32_t timer=30*SECOND; // 30 seconds timeout
    while (apollo_get_zynq_up() == 1 && timer > 0 ) {
      timer = timer - 1;
      if (timer==0)
        ipmc_ios_printf("   > Timeout waiting for zynq good...\r\n");
    }
  }

  // turn off power
  ipmc_ios_printf(" > Disabling 12V Power...\r\n");
  EN_12V_SET_STATE(RESET);

  APOLLO_STARTUP_DONE = 0;
}

void apollo_powerup_sequence () {

  apollo_init_gpios ();

  osDelay(100);

  uint8_t revision=apollo_get_revision();
  uint8_t boot_mode=APOLLO_BOOT_SD;

  if (revision == APOLLO_REV2 || revision == APOLLO_REV2A) {

    // TODO: right now the noshelf is being used as a boot mode selection...
    // this should be returned to the correct behavior eventually
    if (apollo_get_noshelf()) {
      boot_mode = APOLLO_BOOT_JTAG; // JTAG / EMMC
    } else {
      boot_mode = APOLLO_BOOT_SD;
    }
  } else {
    boot_mode = APOLLO_BOOT_SD;
  }

  LED_0_SET_STATE(RESET);
  LED_1_SET_STATE(RESET);

  // make sure everything is off
  //------------------------------------------------------------------------------
  EN_12V_SET_STATE(RESET);
  apollo_set_zynq_en(0);

  ipmc_ios_printf("Powering Up Service Module:\r\n");

  // set boot bins
  //------------------------------------------------------------------------------
  ipmc_ios_printf(" > Setting boot mode to 0x%1X...\r\n", boot_mode);
  apollo_set_zynq_boot_mode (boot_mode);

  // set uart pins
  //------------------------------------------------------------------------------
  uint8_t uart_adr = APOLLO_UART_DISCONNECT;
  ipmc_ios_printf(" > Setting uart adr to 0x%1X...\r\n", uart_adr);
  apollo_set_uart_adr (uart_adr);

  // set jtag chain select pins
  //------------------------------------------------------------------------------
  uint8_t chain_sel=0x0;
  if (revision == APOLLO_REV1) {
    // chain sel has a different meaning in rev0
    chain_sel = 0;
  }
  ipmc_ios_printf(" > Setting jtag chain sel to 0x%1X...\r\n", chain_sel);
  apollo_set_jtag_chain_sel (chain_sel);

  // send esm reset
  //------------------------------------------------------------------------------
  ipmc_ios_printf(" > Resetting ESM...\r\n");
  apollo_esm_reset(25);

  // turn off we on eeprom
  //------------------------------------------------------------------------------
  apollo_set_eeprom_we_n (1);

  // reset i2c sense mux
  //------------------------------------------------------------------------------
  ipmc_ios_printf(" > Resetting MGM I2C Mux...\r\n");
  apollo_sense_reset ();

  osDelay(10);

  // turn on power
  //------------------------------------------------------------------------------
  ipmc_ios_printf(" > Enabling 12V power...\r\n");
  EN_12V_SET_STATE(SET);

  osDelay(100);

  // set zynq enable
  //------------------------------------------------------------------------------
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

    ipmc_ios_printf(" > Waiting for Zynq FPGA...\r\n");
    while (0==apollo_get_fpga_done()) {;}

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
  uart_adr = 0; // 0 == connect to Zynq
  apollo_set_uart_adr (uart_adr);

  ipmc_ios_printf(" > Powerup done\n", uart_adr);

  APOLLO_STARTUP_DONE = 1;

  // zynq timeout
  //------------------------------------------------------------------------------

  ipmc_ios_printf(" > Waiting for Zynq OS...\r\n");

  if (revision == APOLLO_REV2 || revision == APOLLO_REV2A) {

    // Zynq up is from Linux
    ipmc_ios_printf(" > SMRev2: Waiting for Zynq Up..\r\n");

    uint32_t timer=60*SECOND; // timeout
    while (apollo_get_zynq_up() == 0 && timer > 0 ) {

      timer = timer - 1;

      if (timer==0) {
        ipmc_ios_printf("   > Timeout waiting for zynq good to go up...\r\n");

        // TODO
        // turn off power after 2 minutes?
        // or...
        // if you don't hear back, change boot mode to qspi and try again

      }
    }
  } else {
    ipmc_ios_printf(" > SMRev1: not waiting for Zynq Up..\r\n");
  }

  LED_1_SET_STATE(SET);

}
