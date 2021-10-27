
#include <stdint.h>

#include "fw_metadata.h"

#include "head_commit_sha1.h"
#include "fw_identification.h"

/*
 *  Firmware Metadata
 */
__attribute__((section(".fw_metadata")))
metadata_fields_v0_t fw_metadata =
{
	.presence_word             = FW_METADATA_PRESENCE_WORD,
	.metadata_version          = 0x00,
	.firmware_type             = FW_METADATA_TYPE_OPENIPMC_CM7,
	.reserved1                 = 0,
	.image_size                = 0,  // Filled in post processing
	.manufacturer_id           = MANUFACTURER_ID,
	.product_id                = PRODUCT_ID,
	.firmware_revision_major   = FW_VERSION_MAJOR,
	.firmware_revision_minor   = FW_VERSION_MINOR,
	.firmware_revision_aux     = { (HEAD_COMMIT_SHA1>>24)&0xFF, (HEAD_COMMIT_SHA1>>16)&0xFF, (HEAD_COMMIT_SHA1>>8)&0xFF, (HEAD_COMMIT_SHA1)&0xFF },

	.free_space = {0},

	.checksum = 0 // Filled in post processing
};
