#ifndef BOOTLOADER_CTRL
#define BOOTLOADER_CTRL



// Magic word to confirm the presence of the bootolader control data
#define BOOT_CTRL_PRESENCE_WORD  0xD4ED0DC9

// Load mode
#define BOOT_CTRL_LOAD_NONE                    0   // Load nothing, just run
#define BOOT_CTRL_LOAD_ADDR_FROM_EXT_FLASH     1   // Load image present in "load_source_addr" address of external flash
#define BOOT_CTRL_LOAD_BACKUP_FROM_EXT_FLASH   2   // Load image present in address 0 of external flash

// Error code
#define BOOT_CTRL_NO_ERROR   0

// Struct for bootolader control data version 0
typedef struct
{

	uint32_t presence_word;
	uint8_t  ctrl_data_version;  // 0x00 in this case
	uint8_t  load_mode;
	uint8_t  watchdog_time;      // 0 = No watchdog
	uint8_t  error_code;
	uint32_t load_source_addr;
	uint32_t checksum;

}boot_ctrl_v0_t;



#endif
