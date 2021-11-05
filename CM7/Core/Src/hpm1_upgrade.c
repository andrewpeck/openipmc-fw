
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

#include "fw_identification.h"
#include "hpm1_ctrls.h"
#include "fw_metadata.h"
#include "image_ext_flash.h"
#include "head_commit_sha1.h"
#include "bootloader_ctrl.h"
#include "bootloader_tools.h"



#define OPENIPMC_CM7_TEMP_BLOCK 8 // Block in external flash


static void update_CM7_rollback_version(void);

/*
 * Function called during the OpenIPMC HAL initializations.
 * It prepares the HPM1 Upgrade functionality by setting attributes
 */
void hpm1_init(void)
{

	hpm1_global_capabilities = MANUAL_ROLLBACK;
	hpm1_timeouts.inaccessibility_timeout = 2;
	hpm1_timeouts.rollback_timeout = 4;
	hpm1_timeouts.selftest_timeout = 1;
	hpm1_timeouts.upgrade_timeout = 1;


	// Load info for the Bootloader (if installed)
	uint8_t bl_major_ver; // (version info not used here)
	uint8_t bl_minor_ver;
	uint8_t bl_aux_ver[4];
	if( bootloader_is_present( &bl_major_ver, &bl_minor_ver, bl_aux_ver ) )
	{
		hpm1_add_component( 0 ); // Bootloader is Component 0

		hpm1_component_properties[0]->payload_cold_reset_is_required   = false;
		hpm1_component_properties[0]->deffered_activation_is_supported = false;
		hpm1_component_properties[0]->firmware_comparison_is_supported = false;
		hpm1_component_properties[0]->rollback_is_supported = false;
		hpm1_component_properties[0]->backup_cmd_is_required = false;

		hpm1_component_properties[0]->current_firmware_revision_major  = bl_major_ver;
		hpm1_component_properties[0]->current_firmware_revision_minor  = bl_minor_ver;
		hpm1_component_properties[0]->current_firmware_revision_aux[0] = bl_aux_ver[0];
		hpm1_component_properties[0]->current_firmware_revision_aux[1] = bl_aux_ver[1];
		hpm1_component_properties[0]->current_firmware_revision_aux[2] = bl_aux_ver[2];
		hpm1_component_properties[0]->current_firmware_revision_aux[3] = bl_aux_ver[3];

		strncpy( hpm1_component_properties[0]->description_string, "Bootloader", 11 ); // Max of 11 chars + null termination
	}

	// Load info for the OpenIPMC-FW_CM7 (Component 1)
	hpm1_add_component( 1 );

	hpm1_component_properties[1]->payload_cold_reset_is_required   = true;
	hpm1_component_properties[1]->deffered_activation_is_supported = false;
	hpm1_component_properties[1]->firmware_comparison_is_supported = false;
	hpm1_component_properties[1]->rollback_is_supported = true;
	hpm1_component_properties[1]->backup_cmd_is_required = true;

	hpm1_component_properties[1]->current_firmware_revision_major  = FW_VERSION_MAJOR;
	hpm1_component_properties[1]->current_firmware_revision_minor  = FW_VERSION_MINOR;
	hpm1_component_properties[1]->current_firmware_revision_aux[0] = (HEAD_COMMIT_SHA1>>24)&0xFF;
	hpm1_component_properties[1]->current_firmware_revision_aux[1] = (HEAD_COMMIT_SHA1>>16)&0xFF;
	hpm1_component_properties[1]->current_firmware_revision_aux[2] = (HEAD_COMMIT_SHA1>>8)&0xFF;
	hpm1_component_properties[1]->current_firmware_revision_aux[3] = (HEAD_COMMIT_SHA1)&0xFF;

	strncpy( hpm1_component_properties[1]->description_string, "CM7_fw", 7 ); // Max of 11 chars + null termination

	// Check for CM7 firmware in backup area and update the properties if available;
	update_CM7_rollback_version();


}

/*
 * This functions checks if there is some backup available in backup area for CM7 firmware.
 * If so, update the component properties
 */
static void update_CM7_rollback_version(void)
{
	if( hpm1_component_properties[1] == NULL )
		return;

	if( !image_ext_flash_CRC_is_valid(0) )
	{
		hpm1_component_properties[1]->backup_is_available = false;
		return;
	}
	hpm1_component_properties[1]->backup_is_available = true;

	metadata_fields_v0_t metadata_fields;

	// Read metadata
	image_ext_flash_read( 0, FW_METADATA_ADDR, sizeof(metadata_fields), (uint8_t*)&metadata_fields);

	hpm1_component_properties[1]->rollback_firmware_revision_major  = metadata_fields.firmware_revision_major;
	hpm1_component_properties[1]->rollback_firmware_revision_minor  = metadata_fields.firmware_revision_minor;
	hpm1_component_properties[1]->rollback_firmware_revision_aux[0] = metadata_fields.firmware_revision_aux[0];
	hpm1_component_properties[1]->rollback_firmware_revision_aux[1] = metadata_fields.firmware_revision_aux[1];
	hpm1_component_properties[1]->rollback_firmware_revision_aux[2] = metadata_fields.firmware_revision_aux[2];
	hpm1_component_properties[1]->rollback_firmware_revision_aux[3] = metadata_fields.firmware_revision_aux[3];
}


void hpm1_cmd_initiate_backup_cb( uint8_t component_mask )
{
	// TODO: implement backup error status back to HPM1. For testing purposes we just ignore a backup failure
	image_ext_flash_openipmc_CM7_backup();
	update_CM7_rollback_version();
}

void hpm1_cmd_initiate_prepare_cb( uint8_t component_mask )
{
	// Not used
}

void hpm1_cmd_initiate_upload_for_upgrade_cb( uint8_t component_number )
{
	image_ext_flash_open( OPENIPMC_CM7_TEMP_BLOCK );
}

void hpm1_cmd_initiate_upload_for_compare_cb( uint8_t component_number )
{

}

void hpm1_cmd_upload_cb( uint8_t component_number, uint8_t* block_data, uint8_t block_size  )
{
	image_ext_flash_write( block_data, block_size );
}

int hpm1_cmd_upload_finish_cb( uint8_t component_number  )
{
	image_ext_flash_close();

	if( !image_ext_flash_CRC_is_valid( OPENIPMC_CM7_TEMP_BLOCK ) )
		return HPM1_CB_RETURN_CHECKSUM_ERROR;

	return HPM1_CB_RETURN_OK;
}

void hpm1_cmd_activate_cb( void )
{
	if( bootloader_enable() )
	{
		bootloader_schedule_load( BOOT_CTRL_LOAD_ADDR_FROM_EXT_FLASH );
		NVIC_SystemReset();
	}
}

int  hpm1_cmd_manual_rollback_cb ( void )
{
	if( bootloader_enable() )
	{
		if( image_ext_flash_CRC_is_valid( 0 ) ) // Check if there is something valid in the backup area
		{
			bootloader_schedule_load( BOOT_CTRL_LOAD_BACKUP_FROM_EXT_FLASH );
			NVIC_SystemReset();
			return HPM1_CB_RETURN_OK;
		}
		else
		{
			return HPM1_CB_RETURN_ROLLBACK_ERROR;
		}
	}
	else
	{
		return HPM1_CB_RETURN_ROLLBACK_ERROR;
	}
}



