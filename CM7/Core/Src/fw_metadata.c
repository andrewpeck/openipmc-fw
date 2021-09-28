
#include <stdint.h>

#include "fw_metadata.h"

/*
 *  Firmware Metadata
 *
 *  This union is used to force a reservation of FW_METADATA_MAX_SIZE bytes for
 *  FW metadata area and also allow initialization of constants into the same
 *  place.
 */
__attribute__((section(".fw_metadata")))
const union
{
	uint8_t raw_allocation[FW_METADATA_MAX_SIZE]; // Forces the size of FW metadata area.
	metadata_fields_v0_t metadata_fields;         // Fields to be initialized

} fw_metadata =
{
	.metadata_fields =
	{
		.presence_word     = FW_METADATA_PRESENCE_WORD,
		.metadata_version  = 0x00,
		.firmware_type     = FW_METADATA_TYPE_OPENIPMC_CM7,
		.reserved1         = 0,
		.image_size        = 0  // This information will be added by an editing program in the final *.bin file
	}
};
