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
 *   12   | 4 bytes |  Manufacturer ID (IANA code). 20 bit number. The rest MSB must be 0. 0x000000 = unspecified. 0x0FFFFF = reserved
 *   16   | 2 bytes |  Product ID. 0x0000 = unspecified. 0xFFFF = reserved
 *   18   | 1 byte  |  Major Firmware Revision. Integer 0 ~ 127 (IPMI/HPM.1 format)
 *   19   | 1 byte  |  Minor Firmware Revision. BCD from 00 to 99 (two digits) Example: Ver. 1.2.3 -> 0x23  (IPMI/HPM.1 format)
 *   20   | 4bytes  |  Auxiliary Firmware Revision Information. Any 4 bytes of data typically displayed in hex format (IPMI/HPM.1 format)
 */

// Position of metadata into the the image
#define FW_METADATA_ADDR      0x298
#define FW_METADATA_MAX_SIZE  360


// Magic word to confirm the presence of metadata and image
#define FW_METADATA_PRESENCE_WORD  0xF11B9859


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
	uint32_t manufacturer_id;
	uint16_t product_id;
	uint8_t  firmware_revision_major;
	uint8_t  firmware_revision_minor;
	uint8_t  firmware_revision_aux[4];

	uint8_t  free_space[FW_METADATA_MAX_SIZE - 28]; // Enough to allow 360 bytes in the struct. Must be filled with zeros.

	uint32_t checksum; // 32 bit word checksum of the

}metadata_fields_v0_t;


/*
 * IMPORTANT:
 *   - Newer metadata versions must only add fields by using the free space or reserved fields,
 *     so that even older firmwares can interpret it.
 *   - Firmware with a newer metadata version must be able to recognize older ones to avoid
 *     attempts of reading unavailable information.
 *   - Size of struct must always be 360 bytes.
 */


#endif
