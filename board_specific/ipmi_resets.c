#include "ipmi_resets.h"

/* 
 * Functions implementing the cold/warm/graceful resets of the payload (ApolloSM) are
 * defined in this file.
 * 
 * Functions which start with "reply_" prefix are used to indicate that the respective 
 * IPMI reset command is implemented in the firmware, by providing a 0x00 completion code.
 * Please see "do_begin_*_reset" implementations under 
 * openipmc-fw/CM7/Core/Src/openipmc_fru_payload_ipmc_control.c
 * for details. 
 * 
 * Functions which start with "ipml_begin" prefix are used for the actual implementation
 * of the respective reset.
 * 
 * To invoke these resets using ipmitool, you can use the following:
 * ipmitool -H <Shelf Manager IP> -P "" -t <IPMB Address> picmg frucontrol 0 X
 * 
 * where X depends on the type of reset as follows:
 * X = 0 -> cold reset
 * X = 1 -> warm reset
 * X = 2 -> graceful reboot
 */

/* Cold reset. */
void reply_begin_payload_cold_reset(uint8_t* compl_code) {
  /* We reply OK as this functon is implemented. */
  compl_code = 0x00;
}

/** 
 * @brief Cold reset implementation: Cut the 12V power, and launch Apollo power-up sequence. 
 */
void impl_begin_payload_cold_reset() {
  mt_printf("Cold reset of Zynq is requested\r\n");
  
  /* Disable Zynq enable line. */
  mt_printf(" > Disabling Zynq enable line\r\n");
  apollo_set_zynq_en(0);
  osDelay(500);

  /* Disable 12V power. */
  mt_printf(" > Disabling 12V power\r\n");
  EN_12V_SET_STATE(RESET);
  osDelay(500);

  /* Launch the Apollo power-up sequence. */
  mt_printf(" > Launching Apollo power-up sequence\r\n");
  apollo_powerup_sequence();
}

/* Warm reset. */
void reply_begin_payload_warm_reset(uint8_t* compl_code) {
  /* We reply OK as this functon is implemented. */
  compl_code = 0x00;
}

/** 
 * @brief Warm reset implementation: Launch the Apollo power-down sequence and wait for Zynq to
 *        shut down for 30 seconds (otherwise turn the power off), and launch power-up sequence after.
 */
void impl_begin_payload_warm_reset() {
  mt_printf("Warm reset of Zynq is requested\r\n");

  /* Launch power down sequence with 30 seconds timeout. */
  const uint8_t timeout = 30;
  mt_printf(" > Launching Apollo power-down sequence with %us timeout\r\n", timeout);
  PAYLOAD_RESET_SET_STATE(RESET);
  apollo_powerdown_sequence(timeout);

  osDelay(500);

  /* Launch power up sequence. */
  mt_printf(" > Launching Apollo power-up sequence\r\n");
  PAYLOAD_RESET_SET_STATE(SET);
  apollo_powerup_sequence();
}

/* Graceful reboot. */
void reply_begin_payload_graceful_reboot(uint8_t* compl_code) {
  /* We reply OK as this functon is implemented. */
  compl_code = 0x00;
}

/** 
 * @brief Graceful reboot implementation: Launch the Apollo power-down sequence and wait for Zynq to
 *        shut down for 300 seconds (otherwise turn the power off), and launch power-up sequence after.
 */
void impl_begin_payload_graceful_reboot() {
  mt_printf("Graceful reboot of Zynq is requested\r\n");

  /* Launch power down sequence with 300 seconds timeout. */
  const uint8_t timeout = 300;
  mt_printf(" > Launching Apollo power-down sequence with %u seconds timeout\r\n", timeout);
  PAYLOAD_RESET_SET_STATE(RESET);
  apollo_powerdown_sequence(timeout);

  osDelay(500);

  /* Launch Apollo power up sequence. */
  mt_printf(" > Launching Apollo power-up sequence\r\n");
  PAYLOAD_RESET_SET_STATE(SET);
  apollo_powerup_sequence();
}