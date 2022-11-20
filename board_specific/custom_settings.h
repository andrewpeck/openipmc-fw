#ifndef CUSTOM_SETTINGS
#define CUSTOM_SETTINGS


#include "cmsis_os.h"

// https://gitlab.com/openipmc/openipmc-fw-board-specific-demo/-/blob/master/board_specific/custom_settings.h
#define CUSTOM_STARTUP_TASK_STACK_SIZE 256  // In 4byte words
#define CUSTOM_STARTUP_TASK_PRIORITY osPriorityNormal
#define OPENIPMC_FRU_STATE_MACHINE_TASK_STACK_SIZE 512  // In 4byte words
#define OPENIPMC_INCOMING_REQUESTS_TASK_STACK_SIZE 512  // In 4byte words
#define TERMINAL_MAX_NUMBER_OF_COMMANDS 35
#define TERMINAL_PROCESS_TASK_STACK_SIZE 512  // In 4byte words

// Set if DHCP is ON or OFF by default (0 = OFF; 1 = ON)
#define DHCP_STARTUP_STATE 1;

#define CLI_AVAILABLE_ON_UART 0

#endif
