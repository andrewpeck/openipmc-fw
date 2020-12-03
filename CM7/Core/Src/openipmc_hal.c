
/*
 * This file contains the all specific implementation of the OpenIPMC's Hardware
 * Abstraction Layer functions. These functions are all declared in ipmc_ios.h.
 */

// FreeRTOS includes
//#include <stdio.h>
//#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// STM32H7xx HAL includes
#include "main.h"
#include "cmsis_os.h"

// OpenIPMC includes
//#include "openipmc/src/ipmc_ios.h"
//#include "openipmc/src/ipmb_0.h"
//#include "openipmc/src/ipmi_msg_manager.h"
//#include "openipmc/src/fru_state_machine.h"
//#include "openipmc/src/ipmc_tasks.h"

// printf lib include
//#include "../printf/printf.h"

#define ACK_CHECK_EN 0x1       /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0      /*!< I2C master will not check ack from slave */

//#define I2C_SCLK_RATE   100000 // 100 kHz I2C bus
#define I2C_MODE_MASTER 1
#define I2C_MODE_SLAVE  0
#define IPMB_BUFF_SIZE  32

static _Bool ipmc_ios_ready_flag = pdFALSE;

// I2C state control variables in OpenIPMC HAL
static uint8_t i2c1_current_state;
static uint8_t i2c4_current_state;

// Length of the the received message
static uint32_t i2c1_recv_len = 0;
static uint32_t i2c4_recv_len = 0;

// IPMB Hardware Address
static uint8_t  ipmb_addr;

//Buffers for IPMB receiving
uint8_t ipmba_input_buffer[IPMB_BUFF_SIZE] = {0};
uint8_t ipmbb_input_buffer[IPMB_BUFF_SIZE] = {0};

// Semaphores to synchronize the IPMB operations with the I2C transmission
static SemaphoreHandle_t ipmb_rec_semphr = NULL;
static SemaphoreHandle_t ipmba_send_semphr = NULL;
static SemaphoreHandle_t ipmbb_send_semphr = NULL;

// Mutex to avoid printf overlapping.
static SemaphoreHandle_t printf_mutex = NULL;

// Remap I2C peripherals handlers for IPMB channels
I2C_HandleTypeDef hi2c1;  // JUST FOR TEST
I2C_HandleTypeDef hi2c2;  // JUST FOR TEST
#define hi2c_ipmba hi2c1
#define hi2c_ipmbb hi2c2



/*
 * Initializations related to all peripherals involved in the OpenIPMC's
 * Hardware Abstraction Layer.
 *
 * Many initializations are already carried by automatically generated code,
 * under configuration made in .ioc file. This function address additional
 * initializations needed to connect OpenIPMC to peripherals.
 */
int openipmc_hal_init(void)
{

	// Create the semaphores and mutex
	printf_mutex = xSemaphoreCreateMutex();
	ipmb_rec_semphr = xSemaphoreCreateBinary();
	ipmba_send_semphr = xSemaphoreCreateBinary();
	ipmbb_send_semphr = xSemaphoreCreateBinary();

	//ADICIONAR CÓDIGO PARA HABILITAR OS CANAIS

	// Now peripherals are ready and can be used bu OpenIPMC
	ipmc_ios_ready_flag = pdTRUE;
}



/*
 * Check if the IOs are initialized.
 *
 * Returns TRUE if ipmc_ios_init() ran all the initializations successfully.
 */
_Bool ipmc_ios_ready(void)
{
	return ipmc_ios_ready_flag;
}



/*
 * Read the state of the ATCA handle
 *
 * Mechanically, when the handle is CLOSED, the pin is grounded, giving a LOW.
 * When the handle is OPEN, the micro-switch is also open, and a pull-up
 * resistor imposes a HIGH.
 *
 * return value:
 *   1: handle is OPEN
 *   0: handle is CLOSED
 */
int ipmc_ios_read_handle(void)
{
	return 0;
}



/*
 * The Hardware Address of the IPMC (slave addr) is read from the ATCA backplane pins
 * A party check is considered for the address (parity must be ood)
 */
uint8_t ipmc_ios_read_haddress(void)
{
	return 0;
}



/*
 * Set the IPMB address.
 */
void ipmc_ios_ipmb_set_addr(uint8_t addr)
{
	// Convert into 7-bit format to be used by the I2C driver
	ipmb_addr = addr >> 1;

}



/*
 * Master transmit function for IPMB-A
 * IPMB-A uses STM32H7xx I2C1 - set on pins Pxx as SCL and Pxx as SDA
 */
int ipmc_ios_ipmba_send(uint8_t *MsgPtr, int ByteCount)
{
    _Bool semphr_timeout;
    uint16_t dest_addr;
    HAL_StatusTypeDef tx_ret_val;

    dest_addr = (uint16_t)MsgPtr[0]; // Address already shifted

    // Must reconfigure the I2C peripheral before attempting to transmit in master mode
    HAL_I2C_DeInit(&hi2c_ipmba);
    HAL_I2C_Init(&hi2c_ipmba);
	HAL_I2CEx_ConfigAnalogFilter(&hi2c_ipmba, I2C_ANALOGFILTER_ENABLE);
	HAL_I2CEx_ConfigDigitalFilter(&hi2c_ipmba, 0);

    // set current mode as Master
    i2c1_current_state = I2C_MODE_MASTER;

    // begin the transmission
    tx_ret_val = HAL_I2C_Master_Transmit_IT(&hi2c_ipmba, dest_addr, &MsgPtr[1], (uint16_t) ByteCount -1);

    // Wait for transmission to finish or timeout
    semphr_timeout = xSemaphoreTake (ipmba_send_semphr, pdMS_TO_TICKS(100));

    // return I2C to Slave mode
    i2c1_recv_len = 0;
    HAL_I2C_DeInit(&hi2c_ipmba);
    HAL_I2C_Init(&hi2c_ipmba);
    HAL_I2CEx_ConfigAnalogFilter(&hi2c_ipmba, I2C_ANALOGFILTER_ENABLE);
    HAL_I2CEx_ConfigDigitalFilter(&hi2c_ipmba, 0);
    HAL_I2C_Slave_Receive_IT(&hi2c_ipmba, &ipmba_input_buffer[0], IPMB_BUFF_SIZE);
    i2c1_current_state = I2C_MODE_SLAVE;
/*
    if ( (tx_ret_val == HAL_OK) && (semphr_timeout != pdFALSE) )
        return IPMB_SEND_DONE;
    else
    	return IPMB_SEND_FAIL;*/
}



/*
* Master transmit function for IPMB-B
* IPMB-B uses STM32H7xx I2C4 - set on pins Pxx as SCL and Pxx as SDA
*/
int ipmc_ios_ipmbb_send(uint8_t *MsgPtr, int ByteCount)
{
	return 0;
}



int ipmc_ios_ipmba_read(uint8_t *MsgPtr)
{
	return 0;
}



int ipmc_ios_ipmbb_read(uint8_t *MsgPtr )
{
	return 0;
}



/*
 * Holds the ipmb_0_msg_receiver_task until the peripheral task releases the
 * semaphore (it will release when a message is received)
 */
void ipmc_ios_ipmb_wait_input_msg(void)
{
    xSemaphoreTake (ipmb_rec_semphr, portMAX_DELAY);
}



/*
 * Control the Blue Led
 */
void ipmc_ios_blue_led(int blue_led_state)
{
	if (blue_led_state == 0)
	{}//HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, GPIO_PIN_RESET); // LED OFF
	else
	{}//HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, GPIO_PIN_SET); // LED ON
}



/*
 * Implementation of printf. It uses library developed by Marco Paland (info@paland.com)
 * under MIT license available at github.com/mpaland/printf
 */
void ipmc_ios_printf(const char* format, ...)
{
//	va_list args;

//	xSemaphoreTake(printf_mutex, portMAX_DELAY);

//	va_start( args, format );
//	vprintf_( format, args );
//	va_end( args );

//	xSemaphoreGive(printf_mutex);
}


