#ifndef CUSTOM_SETTINGS
#define CUSTOM_SETTINGS


#include "cmsis_os.h"

// https://gitlab.com/openipmc/openipmc-fw-board-specific-demo/-/blob/master/board_specific/custom_settings.h
#define CUSTOM_STARTUP_TASK_STACK_SIZE 256  // In 4byte words
#define CUSTOM_STARTUP_TASK_PRIORITY osPriorityNormal
#define OPENIPMC_FRU_STATE_MACHINE_TASK_STACK_SIZE 512  // In 4byte words
#define OPENIPMC_INCOMING_REQUESTS_TASK_STACK_SIZE 512  // In 4byte words
#define TERMINAL_MAX_NUMBER_OF_COMMANDS 50
#define TERMINAL_PROCESS_TASK_STACK_SIZE 512  // In 4byte words

// Set if DHCP is ON or OFF by default (0 = OFF; 1 = ON)
#define DHCP_STARTUP_STATE 1;

#define CLI_AVAILABLE_ON_UART 0

/*
 * Enable I2C3 and I2C4 peripherals for the h7i2c-baremetal-driver, as those are the 
 * buses used by the Apollo Service Module.
 * Also allow the usage of FreeRTOS compatible implementation.
 */

#define H7I2C_PERIPH_ENABLE_I2C3 1
#define H7I2C_PERIPH_ENABLE_I2C4 1
#define H7I2C_USE_FREERTOS_IMPL  1

#define H7UART_PERIPH_ENABLE_UART5 1

#endif
