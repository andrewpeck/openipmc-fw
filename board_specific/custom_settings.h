#ifndef CUSTOM_SETTINGS
#define CUSTOM_SETTINGS


#include "cmsis_os.h"

#define CUSTOM_STARTUP_TASK_STACK_SIZE 128  // In 4byte words
#define CUSTOM_STARTUP_TASK_PRIORITY osPriorityNormal


// Set if DHCP is ON or OFF by default (0 = OFF; 1 = ON)
#define DHCP_STARTUP_STATE 1;



#endif
