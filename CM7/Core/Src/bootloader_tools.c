
#include "main.h"
#include "fw_metadata.h"
#include "bootloader_ctrl.h"
#include "image_ext_flash.h"

#include "bootloader_tools.h"

#define BOOTLOADER_SECTOR    15   // Sector where the bootloader is placed

#define FLASH_ORIGIN  0x8000000  // First address of Flash in absolute address


__attribute__ ((section (".bootloader_cmd"))) boot_ctrl_v0_t bootloader_ctrl_bckp;



/*Check if bootloader is present and return its version via pointers
 */
bool bootloader_is_present( uint8_t* major_version, uint8_t* minor_version, uint8_t aux_version[4] )
{
	const uint32_t sector_addr     = FLASH_ORIGIN + BOOTLOADER_SECTOR*FLASH_SECTOR_SIZE;

	metadata_fields_v0_t* fw_metadata = (metadata_fields_v0_t*)(sector_addr + FW_METADATA_ADDR);

	// Check presence word
	if( fw_metadata->presence_word != FW_METADATA_PRESENCE_WORD )
		return false;

	// Check metadata checksum
	uint32_t sum = 0;
	for( int i=0; i<(sizeof(metadata_fields_v0_t)/sizeof(uint32_t)); i++ )
		sum += ((uint32_t*)fw_metadata)[i];
	if( sum != 0 )
		return false;

	*major_version = fw_metadata->firmware_revision_major;
	*minor_version = fw_metadata->firmware_revision_minor;
	aux_version[0] = fw_metadata->firmware_revision_aux[0];
	aux_version[1] = fw_metadata->firmware_revision_aux[1];
	aux_version[2] = fw_metadata->firmware_revision_aux[2];
	aux_version[3] = fw_metadata->firmware_revision_aux[3];

	return true;  // Bootloader is present!
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
	uint8_t major; // (version info not used here)
	uint8_t minor;
	uint8_t aux[4];
	if( bootloader_is_present( &major, &minor, aux ) == false )
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

/*
 * Set the bootloader command data in Backaup RAM to perform upload from the TEMP area
 */
void bootloader_schedule_load( uint8_t boot_ctrl_load_mode  )
{
	boot_ctrl_v0_t boot_ctrl;

	boot_ctrl.presence_word = BOOT_CTRL_PRESENCE_WORD;
	boot_ctrl.ctrl_data_version = 0;
	boot_ctrl.load_mode = boot_ctrl_load_mode;
	boot_ctrl.watchdog_time = 0;
	boot_ctrl.error_code = 0;
	boot_ctrl.load_source_addr = 131072 * 8; // If load mode is NOT "BOOT_CTRL_LOAD_ADDR_FROM_EXT_FLASH", this field is ignored by bootloader

	uint32_t sum = 0;
	for( int i = 0; i < (sizeof(boot_ctrl)/4) - 1; i++)
		sum += ((uint32_t*)&boot_ctrl)[i];
	boot_ctrl.checksum = (~sum)+1;

	PWR->CR1 |= PWR_CR1_DBP;

	while((PWR->CR1 & PWR_CR1_DBP) == 0)
		osDelay(5);

	__HAL_RCC_BKPRAM_CLK_ENABLE();

	memcpy( &bootloader_ctrl_bckp, &boot_ctrl, sizeof(boot_ctrl) );
	SCB_CleanDCache_by_Addr((uint32_t*)&bootloader_ctrl_bckp, sizeof(&bootloader_ctrl_bckp));

	PWR->CR1 &= ~PWR_CR1_DBP;

}


