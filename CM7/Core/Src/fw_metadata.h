#ifndef FW_METADATA_H
#define FW_METADATA_H

#include <stdint.h>

/*
 * METADATA FIELD DESCRIPTION
 * Version 0
 *
 * byte#  |   Size  |  Description
 *        |
 *    0   | 4 bytes |  FW_METADATA_IS_PRESENT (Magic Word). LSB first
 *    4   | 1 bytes |  Metadata field version (0x00 in this case)
 *    5   | 1 bytes |  Firmware type. Identify which firmware is this
 *    6   | 2 bytes |  Reserved
 *    8   | 4 bytes |  Image size in bytes. LSB first
 */

// Position of metadata into the the image
#define FW_METADATA_ADDR      0x298
#define FW_METADATA_MAX_SIZE  360


// Magic word to confirm the presence of metadata and image
#define FW_METADATA_PRESENCE_WORD  0xF11B9859


// Constant used by Linux program to insert the image size
#define FW_METADATA_IMAGE_SIZE_OFFSET 8


// Firmware type values
#define FW_METADATA_TYPE_BOOTLOADER_CM7  1
#define FW_METADATA_TYPE_OPENIPMC_CM7    2
#define FW_METADATA_TYPE_OPENIPMC_CM4    3


// Struct for metadata version 0
typedef struct
{
	uint32_t presence_word;
	uint8_t  metadata_version;
	uint8_t  firmware_type;
	uint16_t reserved1;
	uint32_t image_size;

	uint8_t  free_space[FW_METADATA_MAX_SIZE - 16]; // Enough to allow 360 bytes in the struct. Must be filled with zeros.

	uint32_t checksum; // 32 bit word checksum of the

}metadata_fields_v0_t;


/*
 * IMPORTANT:
 *   - New metadata versions must only add fields by using the free space or reserved fields.
 *   - Newer firmware versions must be able to deal with older metadata versions.
 *   - Size of struct must always be 360 bytes.
 */


#endif
