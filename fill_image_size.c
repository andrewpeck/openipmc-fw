


/*
 * This source code generates a program capable to insert the image size
 * into the Firmware Metadata area present in the fw image file (*.bin).
 * 
 * This program must run after the linker genetates the image.
 * 
 * For flexibility, the image path must be passed as argument.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "CM7/Core/Src/fw_metadata.h" 

int main( int argc, char **argv )
{
	metadata_fields_v0_t fw_metadata;
	uint32_t* fw_metadata_32_array = (uint32_t*)(&fw_metadata);
	
	
	if( argc != 2 )
	{
		printf( "Fill FW-Metadata field: Invalid argument." );
		return 1;
	}
	
	FILE* file = fopen( argv[1], "rb+" );
	
	if( file == NULL )
	{
		printf( "Fill FW-Metadata field: No such file or directory." );
		return 1;
	}
	
	// Get the metadata from file
	fseek( file, FW_METADATA_ADDR, SEEK_SET );
	fread( &fw_metadata, sizeof(fw_metadata), 1, file );
	rewind( file );
	
	// Get the image size
	fseek( file, 0, SEEK_END );
	fw_metadata.image_size = ftell( file );
	rewind( file );

	// Calculate checksum
	uint32_t sum = 0;
	for( int i=0; i<(sizeof(fw_metadata)/sizeof(uint32_t))-1; i++ )
		sum += fw_metadata_32_array[i];
	fw_metadata.checksum = (~sum)+1;
	
	// Write Metadata back into the binary
	fseek( file, FW_METADATA_ADDR, SEEK_SET );
	fwrite( &fw_metadata, sizeof(fw_metadata), 1, file );
	
	fclose( file );
	
	return 0;
}
