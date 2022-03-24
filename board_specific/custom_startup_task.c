#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "dimm_gpios.h"
#include "apollo.h"
#include "network_ctrls.h"
#include "ipmc_ios.h"
#include "lwip.h"

void mt_printf(const char* format, ...);

osThreadId_t ipmc_fp_led_blink_task_handle;
const osThreadAttr_t ipmc_fp_led_blink_task_attributes = {
.name = "FP_LED",
.priority = (osPriority_t) osPriorityNormal2,
.stack_size = 128 * 4
};

void openipmc_start( void );
extern void set_benchtop_payload_power_level( uint8_t new_power_level );

void ipmc_fp_led_blink_task( void *argument )
{

	for (;;) {

		if (0==apollo_get_esm_pwr_good()) {

			// ESM power bad

			LED_0_SET_STATE(SET);
			LED_1_SET_STATE(SET);
			LED_2_SET_STATE(SET);
			osDelay(200);
			LED_0_SET_STATE(RESET);
			LED_1_SET_STATE(RESET);
			LED_2_SET_STATE(RESET);
			osDelay(200);

		} else if (ipmc_ios_read_handle() == APOLLO_HANDLE_OPEN) {

			// front panel opened
			LED_0_SET_STATE(SET);
			LED_1_SET_STATE(RESET);
			LED_2_SET_STATE(RESET);
			osDelay(300);
			LED_0_SET_STATE(RESET);
			LED_1_SET_STATE(SET);
			LED_2_SET_STATE(RESET);
			osDelay(300);
			LED_0_SET_STATE(RESET);
			LED_1_SET_STATE(RESET);
			LED_2_SET_STATE(SET);
			osDelay(300);

		} else if (apollo_get_ipmc_abnormal_shutdown() == 1) {

			// bad shutdown
			LED_0_SET_STATE(SET);
			LED_1_SET_STATE(SET);
			LED_2_SET_STATE(SET);
			osDelay(1000);
			LED_0_SET_STATE(RESET);
			LED_1_SET_STATE(RESET);
			LED_2_SET_STATE(RESET);
			osDelay(1000);

		} else if (apollo_get_ipmc_startup_started() == 0) {

			// haven't started powerup... power level is off or something

			LED_1_SET_STATE(RESET);
			LED_2_SET_STATE(RESET);

			LED_0_SET_STATE(SET);
			osDelay(1000);
			LED_0_SET_STATE(RESET);
			osDelay(1000);

		} else if (apollo_get_ipmc_startup_done() == 1) {

			// startup is done

			LED_1_SET_STATE(RESET);
			LED_2_SET_STATE(RESET);

			LED_0_SET_STATE(SET);
			osDelay(100);
			LED_0_SET_STATE(RESET);
			osDelay(100);

		} else if (apollo_get_fpga_done() == 0) {

			// fpga not up

			LED_2_SET_STATE(RESET);

			LED_1_SET_STATE(SET);
			LED_0_SET_STATE(RESET);
			osDelay(1000);
			LED_1_SET_STATE(RESET);
			LED_0_SET_STATE(SET);
			osDelay(1000);

		} else if (apollo_get_zynq_done_generic() == 0) {

			// zynq not up

			LED_2_SET_STATE(RESET);

			LED_1_SET_STATE(SET);
			LED_0_SET_STATE(RESET);
			osDelay(500);
			LED_1_SET_STATE(RESET);
			LED_0_SET_STATE(SET);
			osDelay(500);
		} else {
			LED_1_SET_STATE(RESET);
			LED_2_SET_STATE(RESET);

			LED_0_SET_STATE(SET);
			osDelay(3000);
			LED_0_SET_STATE(RESET);
			osDelay(3000);
		}
	}
}

/*
 * This task is launched by OpenIPMC-FW to allow user to run their own code or
 * launch any other desired task.
 * 
 * The priority and stack size allocated for this task can be controlled in
 * custom_settings.h
 */
void custom_startup_task( void )
{

  apollo_init_gpios();
  apollo_init_bootmode();

  LED_0_SET_STATE(SET);
  LED_1_SET_STATE(SET);
  LED_2_SET_STATE(SET);

  ipmc_fp_led_blink_task_handle = osThreadNew(ipmc_fp_led_blink_task, NULL, &ipmc_fp_led_blink_task_attributes);

  // Check for Benchtop mode
  if (0x41 == ipmc_ios_read_haddress()) {
    mt_printf("1U Shelf Detected... booting up in no shelf mode\r\n");
    set_benchtop_payload_power_level(2);
  }

	eth_ctrls_dhcp_disable();
  // Set network interface static IP Address
  // const uint8_t ip_octet = ipmc_ios_read_haddress();
  const uint8_t ip_octet = 65;
  eth_ctrls_change_ip_addr( 192, 168,  21, ip_octet,  // IP Address
                            255, 255,   0,  0,        // Network Mask
                            192, 168,   1,  4);       // Gateway

  // send esm reset
  //------------------------------------------------------------------------------
  // WARNING:
  //
  //    enabling this ESM reset breaks network access for the IPMC
  //    it needs to be further investigated but for now DO NOT reset the ESM

  mt_printf(" > Resetting ESM...\r\n");
  while (0==apollo_get_esm_pwr_good()) {}

  // for rev1 boards, enabling the esm is tied to enabling the 12V
  // so keep the network off until the ESM comes up and resets
  if (apollo_get_revision() == APOLLO_REV1) {
    while (0==apollo_get_zynq_en()) {}
    osDelay(1000);
  }

  osDelay(100);

	// FIXME!!!! is this ok??
  //MX_LWIP_Init();

	/*
	 * User can choose when the OpenIPMC is started. If any OpenIPMC
	 * board-specific hook/callback requires some previous code to run,
	 * it must be done before this point.
	 */
	openipmc_start();

	for(;;)
	{
		osDelay (30000); // once per 30s
    apollo_write_zynq_i2c_constants ();
	}

}
