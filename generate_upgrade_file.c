

/*
 * This program generates the HPM1 upgrade file for the CM7 firmware of OpenIPMC-FW project
 * 
 * For flexibility, the *.bin file must be passed as argument to this program
 * 
 * NOTE: For compiling, include flags: "-lcrypto -lssl -lz"
 *     e.g.: 
 *        gcc generate_upgrade_file.c -o generate_upgrade_file  -lcrypto -lssl -lz
 */



#include <stdio.h> 
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <openssl/md5.h>
#include <zlib.h>
#include <unistd.h>



#define COMPONENT_MASK_0    0x01
#define COMPONENT_MASK_1    0x02
#define COMPONENT_MASK_2    0x04

// Image Capabilities flags
#define IM_CAP_SELF_TEST_IS_SUPPORTED         0x01
#define IM_CAP_AUTO_ROLLBACK_IS_SUPPORTED     0x02
#define IM_CAP_MANUAL_ROLLBACK_IS_SUPPORTED   0x04
#define IM_CAP_PYLD_OR_FRU_COULD_BE_AFFECTED  0x08

#define FILE_NAME "upgrade.hpm"



typedef struct
{
	uint8_t  major;  // 7 bits (0 ~ 127)
	uint8_t  minor;  // BCD from 00 to 99. (Example: Ver. x.2.3 -> 0x23)
	uint32_t aux;    // Any 32bit data
	
} revision_t;


typedef struct
{
	
	uint32_t   manufacturer_id;           // IANA number
	uint16_t   product_id;                // Manufacturer defined code 
	uint32_t   time;                      // Creation time of this image in seconds since 00:00:00 UTC on January 1, 1970
	uint8_t    image_capabilities;        // A combination of "Image Capabilities flags". See above.
	uint8_t    components;                // Mask informing the components present in this image
	
	uint8_t    self_test_timeout;
	uint8_t    rollback_timeout;
	uint8_t    inaccessibility_timeot;
	
	uint8_t    earliest_compatible_major_revision; // 7 bits (0 ~ 127)
	uint8_t    earliest_compatible_minor_revision; // BCD from 00 to 99. (Example: Ver. x.2.3 -> 0x23)
	
	revision_t firmware_revision;  // Version of the firmware contained in this upgrade file
	
} image_header_t;





void add_image_header(FILE* file, image_header_t header);
void add_backup_componets_action(FILE* file, uint8_t component_mask);
void add_prepare_componets_action(FILE* file, uint8_t component_mask);
void add_upload_firmware_image_action(FILE* file, uint8_t component_number, revision_t version, char* description_string, uint8_t* image_data, uint32_t image_size);

uint8_t* load_binary_from_file(char* file_name, uint32_t* size);

void append_md5(char* file_name);





int main( int argc, char **argv )
{

	image_header_t image_header;
	char* image_file_name;
	
	if( argc != 2 )
	{
		printf( "Generate Upgrade File: Invalid argument." );
		return 1;
	}
	image_file_name = argv[1]; 
	
	// Fill the header info
	image_header.manufacturer_id                    = 0x315A;
	image_header.product_id                         = 0x00;
	image_header.time                               = 0x00;
	image_header.image_capabilities                 = IM_CAP_PYLD_OR_FRU_COULD_BE_AFFECTED |
	                                                  IM_CAP_MANUAL_ROLLBACK_IS_SUPPORTED;
	
	image_header.components                         = COMPONENT_MASK_0;
	
	image_header.self_test_timeout                  = 0x02; // 10 seconds
	image_header.rollback_timeout                   = 0x02; // 10 seconds
	image_header.inaccessibility_timeot             = 0xFF; // 20 seconds
	
	image_header.earliest_compatible_major_revision = 0;    // Any version is accepted
	image_header.earliest_compatible_minor_revision = 0x00;
	
	revision_t version = { 1, 0x22, 0xbebacafe };
	
	image_header.firmware_revision = version;
	
    
    
    
	FILE* file = fopen( FILE_NAME,"wb" );
	
	add_image_header(file, image_header);
	
	add_backup_componets_action(file, COMPONENT_MASK_0);
	
	add_prepare_componets_action(file, COMPONENT_MASK_0);
	
	// Load binary for component 0
	uint8_t* component0_binary_data;
	uint32_t component0_binary_size;
	component0_binary_data = load_binary_from_file( image_file_name, &component0_binary_size );
	if( component0_binary_data == NULL )
	{
		printf( "Generate Upgrade File: Image file does not exist." );
		return 1;
	}
	
	add_upload_firmware_image_action( file, 0, version, "my_comp", component0_binary_data, component0_binary_size );
	
	free( component0_binary_data );
	
	fclose(file);
	
	append_md5( FILE_NAME ); // File is reopened inside
	
}








void add_image_header(FILE* file, image_header_t header)
{
	
	const int fixed_size = 34;      // Header has 34 fixed bytes + OEM + checksum
	uint8_t image_header_array[34];
	
	// Fill the Image Header
	memcpy( &(image_header_array[0]), "PICMGFWU", 8 );  // Signature
	image_header_array[ 8] = 0x00;                      // Format Version
	image_header_array[ 9] = 0x00;                      // Device ID for ATCA IPMC is always 0
	memcpy( &(image_header_array[10]), &header.manufacturer_id, 3 );	
	memcpy( &(image_header_array[13]), &header.product_id, 2 );
	memcpy( &(image_header_array[15]), &header.time, 4 );
	image_header_array[19] = header.image_capabilities;
	image_header_array[20] = header.components;
	image_header_array[21] = header.self_test_timeout;
	image_header_array[22] = header.rollback_timeout;
	image_header_array[23] = header.inaccessibility_timeot;
	image_header_array[24] = header.earliest_compatible_major_revision;
	image_header_array[25] = header.earliest_compatible_minor_revision;
	image_header_array[26] = header.firmware_revision.major;
	image_header_array[27] = header.firmware_revision.minor;
	image_header_array[28] = (header.firmware_revision.aux >> 24) & 0xFF; // Big-endian (not Intel)
	image_header_array[29] = (header.firmware_revision.aux >> 16) & 0xFF;
	image_header_array[30] = (header.firmware_revision.aux >>  8) & 0xFF;
	image_header_array[31] = (header.firmware_revision.aux >>  0) & 0xFF;
	image_header_array[32] = 0; //No OEM data
	image_header_array[33] = 0;
	
	fwrite( image_header_array, 1, fixed_size, file );
	
	// Calculate checksum 
	uint8_t header_sum = 0;
	for( int i=0; i<fixed_size; i++)
		header_sum += image_header_array[i];
	uint8_t checksum = (~header_sum)+1; // Checksum
	
	fwrite( &checksum, 1, 1, file );
	
	
}

void add_backup_componets_action(FILE* file, uint8_t component_mask)
{
	uint8_t action[3];
	action[0] = 0x00;  // Action type: BACKUP
	action[1] = component_mask;
	action[2] = (~(action[0] + action[1]))+1;
	
	fwrite( action, 1, 3, file );
}


void add_prepare_componets_action(FILE* file, uint8_t component_mask)
{
	uint8_t action[3];
	action[0] = 0x01; // Action type: PREPARE
	action[1] = component_mask;
	action[2] = (~(action[0] + action[1]))+1;
	
	fwrite( action, 1, 3, file );
}


void add_upload_firmware_image_action(FILE* file, uint8_t component_number, revision_t version, char* description_string, uint8_t* image_data, uint32_t image_size)
{
	//Add Action: Upload Component 1
	uint8_t action[34];
	action[0] = 0x02;  // Action type: UPLOAD IMAGE
	action[1] = 0x01 << component_number;
	action[2] = (~(action[0] + action[1]))+1;
	
	action[3] =  version.major;
	action[4] =  version.minor;
	action[5] = (version.aux >> 24) & 0xFF; // Big-endian (not Intel)
	action[6] = (version.aux >> 16) & 0xFF;
	action[7] = (version.aux >>  8) & 0xFF;
	action[8] = (version.aux >>  0) & 0xFF;
	
	// Copy string limited in 20 chars + end-of-string
	for(int i=0; i<21; i++) action[9+i] = 0;
	for(int i=0; i<20; i++)
	{
		if(description_string[i] != 0)
			action[9+i] = description_string[i];
		else
			break;
	}
	
	*(uint32_t*)(&action[30]) = image_size + 4; // Include 4 bytes for crc32
	
	// Fill with header
	fwrite( action, 1, 34, file );
	
	// Fill with image data
	fwrite( image_data, 1, image_size, file );
	
	// Calculate CRC32 and add to the beginning of the image
	uLong long_crc = crc32(0L, Z_NULL, 0);
	long_crc = crc32(long_crc, image_data, image_size);
	uint32_t crc = long_crc & 0xFFFFFFFF;
	fwrite( &crc, 4, 1, file );
	
	printf("crc32: %x\n", crc);
	
	// Test CRC32
	//printf("crc32: %lx\n", crc);
	//FILE* filecrc = fopen( "test_crc","wb" );
	//fwrite( image_data, 1, image_size, filecrc );
	//fclose(filecrc);
	
}

uint8_t* load_binary_from_file(char* file_name, uint32_t* size)
{
	FILE* file = fopen( file_name,"rb" );
	if( file == NULL ) return NULL;
	
	// Get size
	fseek( file, 0L, SEEK_END );
	*size = ftell( file );
	rewind( file );
	
	uint8_t* data = malloc( *size );
	fread( data, 1, *size, file );
	
	fclose( file );
	
	return data;
}


void append_md5(char* file_name)
{
	// Calculates md5 sum
	MD5_CTX ctx;
	uint8_t data;
	uint8_t md5_sum[16];
	
	MD5_Init( &ctx );
	FILE* file = fopen( file_name,"rb" );
	while ( fread( &data, 1, 1, file ) == 1 )
		MD5_Update( &ctx, &data, 1 );
	
	fclose(file);
	MD5_Final( md5_sum, &ctx );
	
	// Append md5 to the file 
	file = fopen( file_name,"ab+" );
	fwrite(md5_sum, 1, 16, file);
	fclose(file);
}


