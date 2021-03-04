

/*
 * This module allows to  launche the bootloader provided by
 * STMicroelectronics. It is located in the System Memory.
 *
 * This is useful for loading new firmware via USB (DFU), SPI,
 * I2C, etc.
 *
 * For more information see AN2606 (STM32 microcontroller system
 * memory boot mode).
 */


#include "main.h"

#define     SCHEDULED  1
#define NOT_SCHEDULED  0

/*
 * This flag must not be initialized with ZERO
 *
 * 32 bits are used to ensure that any garbage will match SCHEDULED value
 */
__attribute__ ((section (".noinit"))) static uint32_t jump_to_bootloader;


/*
 * This function launches the bootloader
 *
 * Jump to the bootloader generally requires many peripherals and other
 * resources to be "re-configured" to the boot-up state. In order to simplify
 * this precess, a flag is used to signalizes the jump during the startup, so
 * System Reset is issued to re-start the program.
 */
void st_bootloader_launch(void)
{
	jump_to_bootloader = SCHEDULED;

	NVIC_SystemReset();
}



/*
 * This function forces a jump into the bootloader.
 *
 * It must be called as early as possible during the startup. If
 * "jump_to_bootloader" is SCHEDULED, it performs the jump
 */
void st_bootloader_jump_if_scheduled(void)
{
  uint32_t i=0;
  void (*SysMemBootJump)(void);

  if( jump_to_bootloader != SCHEDULED )
	  return;

  // Un-schedule this jump for next reboot
  jump_to_bootloader = NOT_SCHEDULED;

  /* Set the address of the entry point to bootloader */
     volatile uint32_t BootAddr = 0x1FF09800;

  /* Disable all interrupts */
     __disable_irq();

  /* Disable Systick timer */
     SysTick->CTRL = 0;

  /* Set the clock to the default state */
     HAL_RCC_DeInit();

  /* Clear Interrupt Enable Register & Interrupt Pending Register */
     for (i=0;i<5;i++)
     {
	  NVIC->ICER[i]=0xFFFFFFFF;
	  NVIC->ICPR[i]=0xFFFFFFFF;
     }

  /* Re-enable all interrupts */
     __enable_irq();

  /* Set up the jump to booloader address + 4 */
     SysMemBootJump = (void (*)(void)) (*((uint32_t *) ((BootAddr + 4))));

  /* Set the main stack pointer to the bootloader stack */
     __set_MSP(*(uint32_t *)BootAddr);

  /* Call the function to jump to bootloader location */
     SysMemBootJump();

  /* Jump is done successfully */
     while (1)
     {
      /* Code should never reach this loop */
     }
}
