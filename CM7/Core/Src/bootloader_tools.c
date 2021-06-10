
#include "main.h"
#include "bootloader_tools.h"

#define BOOTLOADER_SECTOR    15   // Sector where the bootloader is placed

#define FLASH_ORIGIN  0x8000000  // First address of Flash in absolute address

/*
 * Magic words to check if bootloader is present
 *
 * These words are the same used by the bootloader to perform the jump to
 * the target firmware (see run_firmware.c in the bootloader code).
 */
static const uint32_t bootloader_presence_numbers[4] =
{
	0xC878A2A0,
	0x1CEE78CC,
	0xD4ED0DC9,
	0x3CAEB661
};


/*
 * Search for the magic word inside the bootloader binary
 */
bool bootloader_is_present( void )
{
	const uint32_t sector_addr     = FLASH_ORIGIN + BOOTLOADER_SECTOR*FLASH_SECTOR_SIZE;
	const uint32_t sector_addr_max = sector_addr + FLASH_SECTOR_SIZE - 4;

	for( uint32_t i = sector_addr; i < sector_addr_max; i += 4 )
	{
		uint32_t* word = (uint32_t*)i;

		if( word[0] == bootloader_presence_numbers[0] &&
		    word[1] == bootloader_presence_numbers[1] &&
		    word[2] == bootloader_presence_numbers[2] &&
		    word[3] == bootloader_presence_numbers[3]    )
		{
			return true; // Bootloader is present!
		}
	}

	return false;  // Bootloader is NOT present!
}


/*
 * Checks if the STM32 CM7 boot address is pointing to the Bootloader
 */
bool bootloader_is_active( void )
{
	FLASH_OBProgramInitTypeDef pOBInit;

	const uint32_t bootloader_sector_addr = FLASH_ORIGIN + BOOTLOADER_SECTOR*FLASH_SECTOR_SIZE;

	HAL_FLASHEx_OBGetConfig( &pOBInit );

	if( pOBInit.BootAddr0 == bootloader_sector_addr )
		return true;
	else
		return false;

}


/*
 * Sets the STM32 CM7 boot address to the Bootloader
 */
bool bootloader_enable( void )
{
	FLASH_OBProgramInitTypeDef pOBInit;

	const uint32_t bootloader_sector_addr = FLASH_ORIGIN + BOOTLOADER_SECTOR*FLASH_SECTOR_SIZE;

	// Be sure that bootloader is present in the flash
	if( bootloader_is_present() == false )
		return false; // Abort

	HAL_FLASHEx_OBGetConfig( &pOBInit );

	if( pOBInit.BootAddr0 == bootloader_sector_addr )
		return true; // Already enabled

	pOBInit.OptionType = OPTIONBYTE_BOOTADD;
	pOBInit.BootConfig = OB_BOOT_ADD0;
	pOBInit.BootAddr0 = bootloader_sector_addr;

	HAL_FLASH_OB_Unlock();
	HAL_FLASHEx_OBProgram( &pOBInit );
	HAL_FLASH_OB_Launch();
	HAL_FLASH_OB_Lock();

	// Cross check!
	HAL_FLASHEx_OBGetConfig( &pOBInit );

	if( pOBInit.BootAddr0 == bootloader_sector_addr )
		return true;
	else
		return false;
}


/*
 * Sets the STM32 CM7 boot address to the Default (Addr 0x08000000, Flash origin)
 *
 * In this case STM32 boots directly into OpenIPMC-FW
 */
bool bootloader_disable( void )
{
	FLASH_OBProgramInitTypeDef pOBInit;

	const uint32_t default_sector_addr = FLASH_ORIGIN;

	HAL_FLASHEx_OBGetConfig( &pOBInit );

	if( pOBInit.BootAddr0 == default_sector_addr )
		return true; // Already disabled

	pOBInit.OptionType = OPTIONBYTE_BOOTADD;
	pOBInit.BootConfig = OB_BOOT_ADD0;
	pOBInit.BootAddr0  = default_sector_addr;

	HAL_FLASH_OB_Unlock();
	HAL_FLASHEx_OBProgram( &pOBInit );
	HAL_FLASH_OB_Launch();
	HAL_FLASH_OB_Lock();


	// Cross check!
	HAL_FLASHEx_OBGetConfig( &pOBInit );

	if( pOBInit.BootAddr0 == default_sector_addr )
		return true;
	else
		return false;
}




