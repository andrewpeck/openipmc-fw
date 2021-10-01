
#include <stdint.h>

#include "fw_metadata.h"

/*
 *  Firmware Metadata
 */
__attribute__((section(".fw_metadata")))
metadata_fields_v0_t fw_metadata =
{
	.presence_word     = FW_METADATA_PRESENCE_WORD,
	.metadata_version  = 0x00,
	.firmware_type     = FW_METADATA_TYPE_OPENIPMC_CM7,
	.reserved1         = 0,
	.image_size        = 0,  // Filled in post processing

	.free_space = {0},

	.checksum = 0 // Filled in post processing
};
