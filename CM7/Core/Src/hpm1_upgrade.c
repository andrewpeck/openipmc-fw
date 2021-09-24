
/********************************************************************************/
/*                                                                              */
/*    OpenIPMC-FW                                                               */
/*    Copyright (C) 2020-2021 Andre Cascadan, Luigi Calligaris                  */
/*                                                                              */
/*    This program is free software: you can redistribute it and/or modify      */
/*    it under the terms of the GNU General Public License as published by      */
/*    the Free Software Foundation, either version 3 of the License, or         */
/*    (at your option) any later version.                                       */
/*                                                                              */
/*    This program is distributed in the hope that it will be useful,           */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*    GNU General Public License for more details.                              */
/*                                                                              */
/*    You should have received a copy of the GNU General Public License         */
/*    along with this program.  If not, see <https://www.gnu.org/licenses/>.    */
/*                                                                              */
/********************************************************************************/

/*
 * This file implements the hardware specific operations required by HPM1 Upgrade
 * in OpenIPMC submodule
 */

#include <stdbool.h>
#include <string.h>
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"

#include "hpm1_ctrls.h"
#include "image_ext_flash.h"


/*
 * Function called during the OpenIPMC HAL initializations.
 * It prepares the HPM1 Upgrade functionality by setting attributes
 */
void hpm1_init(void)
{

	hpm1_global_capabilities = 0;
	hpm1_timeouts.inaccessibility_timeout = 1;
	hpm1_timeouts.rollback_timeout = 1;
	hpm1_timeouts.selftest_timeout = 1;
	hpm1_timeouts.upgrade_timeout = 1;

	hpm1_add_component( 0 );

	hpm1_component_properties[0]->payload_cold_reset_is_required   = false;
	hpm1_component_properties[0]->deffered_activation_is_supported = false;
	hpm1_component_properties[0]->firmware_comparison_is_supported = false;
	hpm1_component_properties[0]->rollback_is_supported = true;
	hpm1_component_properties[0]->backup_cmd_is_required = true;

	hpm1_component_properties[0]->current_firmware_revision_major = 1;
	hpm1_component_properties[0]->current_firmware_revision_minor = 0x23;
	hpm1_component_properties[0]->current_firmware_revision_aux   = 0x12abcdef;

	strncpy( hpm1_component_properties[0]->description_string, "CM7_fw", 12 );

	hpm1_component_properties[0]->rollback_firmware_revision_major = 0x5;
	hpm1_component_properties[0]->rollback_firmware_revision_minor = 0x67;
	hpm1_component_properties[0]->rollback_firmware_revision_aux   = 0x12000000;

	hpm1_component_properties[0]->backup_is_available = false;
}


void hpm1_cmd_initiate_backup_cb( uint8_t component_mask )
{
	mt_printf("BACKUP 0x%x\r\n", component_mask);
}

void hpm1_cmd_initiate_prepare_cb( uint8_t component_mask )
{
	mt_printf("PREPARE 0x%x\r\n", component_mask);
}

void hpm1_cmd_initiate_upload_for_upgrade_cb( uint8_t component_number )
{
	mt_printf("INIT UPLOAD UPGRADE %d\r\n", component_number);

	image_ext_flash_open( 0 );
}

void hpm1_cmd_initiate_upload_for_compare_cb( uint8_t component_number )
{
	mt_printf("INIT UPLOAD COMPARE %d\r\n", component_number);

}

void hpm1_cmd_upload_cb( uint8_t component_number, uint8_t* block_data, uint8_t block_size  )
{
	mt_printf("UPLOAD IMAGE %d\r\n", component_number);
	for (int i = 0; i<block_size; i++) mt_printf("%d ", block_data[i]);
	mt_printf("\r\n");
	//vTaskDelay( pdMS_TO_TICKS(100) );

	image_ext_flash_write( block_data, block_size );

}

void hpm1_cmd_upload_finish_cb( uint8_t component_number  )
{
	mt_printf("FINISH UPLOAD %d\r\n", component_number);
	image_ext_flash_close();
}

void hpm1_cmd_activate_cb( void )
{
	mt_printf("ACTIVATE \r\n");
}



