

#include "cmsis_os.h"
#include "dimm_gpios.h"

void openipmc_start( void );

/*
 * This task is launched by OpenIPMC-FW to allow user to run their own code or
 * launch any other desired task.
 * 
 * The priority and stack size allocated for this task can be controlled in
 * custom_settings.h
 */
void custom_startup_task( void )
{


	/*
	 * User can choose when the OpenIPMC is started. If any OpenIPMC
	 * board-specific hook/callback requires some previous code to run,
	 * it must be done before this point.
	 */
	openipmc_start();


	for(;;)
	{
		// Blink led
		osDelay(500);
		LED_2_SET_STATE(SET);
		osDelay(500);
		LED_2_SET_STATE(RESET);
	}

}
