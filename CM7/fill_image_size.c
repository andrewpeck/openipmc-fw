


/*
 * This source code generates a simple program capable to insert the image size
 * int the Firmware Metadata area present in the fw image file (*.bin).
 * 
 * This program must run after the linker genetates the image.
 * 
 * For flexibility, the image path must be passed as argument.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "Core/Src/fw_metadata.h" 

int main( int argc, char **argv )
{
	uint32_t image_size;
	
	
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
	
	// Get the image size
	fseek( file, 0, SEEK_END );
	image_size = ftell( file );
	rewind( file );

	// Write Metadata into the binary
	fseek( file, FW_METADATA_ADDR + FW_METADATA_IMAGE_SIZE_OFFSET, SEEK_SET );
	fwrite( &image_size, sizeof(image_size), 1, file );
	
	fclose( file );
	
	return 0;
}
