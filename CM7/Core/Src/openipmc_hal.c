
/*
 * This file contains all specific implementation of the OpenIPMC's Hardware
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
#include "ipmc_ios.h"
//#include "openipmc/src/ipmb_0.h"
//#include "openipmc/src/ipmi_msg_manager.h"
//#include "openipmc/src/fru_state_machine.h"
//#include "openipmc/src/ipmc_tasks.h"

#include "dimm_gpios.h"

// printf lib include
#include "printf.h"


#define I2C_MODE_MASTER 1
#define I2C_MODE_SLAVE  0
#define IPMB_BUFF_SIZE  32

static _Bool ipmc_ios_ready_flag = pdFALSE;

// I2C state control variables in OpenIPMC HAL
static uint8_t i2c_ipmba_current_state;
static uint8_t i2c_ipmbb_current_state;

// Length of the the received message
static uint32_t i2c_ipmba_recv_len = 0;
static uint32_t i2c_ipmbb_recv_len = 0;

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

// Re-map I2C peripherals handlers for IPMB channels
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
#define hi2c_ipmba hi2c1
#define hi2c_ipmbb hi2c2

// Re-map UART peripherals handlers for printing outputs
extern UART_HandleTypeDef huart4;
#define huart4_printout huart4


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

	// Create the semaphores and mutexes
	printf_mutex = xSemaphoreCreateMutex();
	ipmb_rec_semphr = xSemaphoreCreateBinary();
	ipmba_send_semphr = xSemaphoreCreateBinary();
	ipmbb_send_semphr = xSemaphoreCreateBinary();

	// Enable the IPMB channels
	GPIO_SET_STATE(SET, IPMB_A_EN);
	GPIO_SET_STATE(SET, IPMB_B_EN);
	osDelay(100);
	if(IPMB_A_RDY_GET_STATE() && IPMB_B_RDY_GET_STATE())
		ipmc_ios_printf("IPMB Decouplers Ready\n\r");
	else
		ipmc_ios_printf("IPMB Decouplers NOT Ready\n\r");


	// Start Receiving on IPMB
	HAL_I2C_Slave_Receive_IT(&hi2c_ipmba, &ipmba_input_buffer[0], IPMB_BUFF_SIZE);
	HAL_I2C_Slave_Receive_IT(&hi2c_ipmbb, &ipmbb_input_buffer[0], IPMB_BUFF_SIZE);
	i2c_ipmba_current_state = I2C_MODE_SLAVE;
	i2c_ipmbb_current_state = I2C_MODE_SLAVE;

	// Now peripherals are ready and can be used bu OpenIPMC
	ipmc_ios_ready_flag = pdTRUE;

	return 0;
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
	if( HANDLE_GET_STATE() == GPIO_PIN_RESET ) // LOW = LOCKED/CLOSED
		return 0;
	else                                       // HIGH = UNLOCKED/OPEN
		return 1;
}



/*
 * The Hardware Address of the IPMC (slave addr) is read from the ATCA backplane pins
 * A party check is considered for the address (parity must be ood)
 */
uint8_t ipmc_ios_read_haddress(void)
{
	int i;
	uint8_t  HA_bit[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t  HA_num;
	int parity_odd;

	// Get HA0 to HA7
	if( HW_0_GET_STATE() == GPIO_PIN_SET )
		HA_bit[0] = 1;
	if( HW_1_GET_STATE() == GPIO_PIN_SET )
		HA_bit[1] = 1;
	if( HW_2_GET_STATE() == GPIO_PIN_SET )
		HA_bit[2] = 1;
	if( HW_3_GET_STATE() == GPIO_PIN_SET )
		HA_bit[3] = 1;
	if( HW_4_GET_STATE() == GPIO_PIN_SET )
		HA_bit[4] = 1;
	if( HW_5_GET_STATE() == GPIO_PIN_SET )
		HA_bit[5] = 1;
	if( HW_6_GET_STATE() == GPIO_PIN_SET )
		HA_bit[6] = 1;
	if( HW_7_GET_STATE() == GPIO_PIN_SET )
		HA_bit[7] = 1;


	/* Calculate parity */
	parity_odd = 0; // initialize as EVEN
	for(i=0; i<8; i++)
		if(HA_bit[i] == 1)
			parity_odd = ~parity_odd; // flip parity

	/* Result */
	HA_num = 0;
	if( parity_odd )
	{
		for(i=0; i<=6; i++)
			HA_num |= (HA_bit[i]<<i);

		return HA_num; // 7bit addr
	}
	else
		return HA_PARITY_FAIL; //parity fail (must be ODD)
}



/*
 * Set the IPMB address.
 */
void ipmc_ios_ipmb_set_addr(uint8_t addr)
{
	// Convert into 7-bit format to be used by the I2C driver
	ipmb_addr = addr >> 1;

	// Set I2C address
	HAL_I2C_DeInit(&hi2c_ipmba);
	hi2c_ipmba.Init.OwnAddress1 = (0x300 | ipmb_addr << 1);
	HAL_I2C_Init(&hi2c_ipmba);

	HAL_I2C_DeInit(&hi2c_ipmbb);
	hi2c_ipmbb.Init.OwnAddress1 = (0x300 | ipmb_addr << 1);
	HAL_I2C_Init(&hi2c_ipmbb);

	// Start Receiving on IPMB
	HAL_I2C_Slave_Receive_IT(&hi2c_ipmba, &ipmba_input_buffer[0], IPMB_BUFF_SIZE);
	HAL_I2C_Slave_Receive_IT(&hi2c_ipmbb, &ipmbb_input_buffer[0], IPMB_BUFF_SIZE);
	i2c_ipmba_current_state = I2C_MODE_SLAVE;
	i2c_ipmbb_current_state = I2C_MODE_SLAVE;
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
	i2c_ipmba_current_state = I2C_MODE_MASTER;

	// begin the transmission
	tx_ret_val = HAL_I2C_Master_Transmit_IT(&hi2c_ipmba, dest_addr, &MsgPtr[1], (uint16_t) ByteCount -1);
	//tx_ret_val = HAL_I2C_Master_Transmit(&hi2c_ipmba, dest_addr, &MsgPtr[1], (uint16_t) ByteCount -1, 2000);

	// Wait for transmission to finish or timeout
	semphr_timeout = xSemaphoreTake (ipmba_send_semphr, pdMS_TO_TICKS(100));

	// return I2C to Slave mode
	i2c_ipmba_recv_len = 0;
	HAL_I2C_DeInit(&hi2c_ipmba);
	HAL_I2C_Init(&hi2c_ipmba);
	HAL_I2CEx_ConfigAnalogFilter(&hi2c_ipmba, I2C_ANALOGFILTER_ENABLE);
	HAL_I2CEx_ConfigDigitalFilter(&hi2c_ipmba, 0);
	HAL_I2C_Slave_Receive_IT(&hi2c_ipmba, &ipmba_input_buffer[0], IPMB_BUFF_SIZE);
	i2c_ipmba_current_state = I2C_MODE_SLAVE;

	if ( (tx_ret_val == HAL_OK) && (semphr_timeout != pdFALSE) )
		return IPMB_SEND_DONE;
	else
		return IPMB_SEND_FAIL;
}



/*
* Master transmit function for IPMB-B
* IPMB-B uses STM32H7xx I2C4 - set on pins Pxx as SCL and Pxx as SDA
*/
int ipmc_ios_ipmbb_send(uint8_t *MsgPtr, int ByteCount)
{
	_Bool semphr_timeout;
	uint16_t dest_addr;
	HAL_StatusTypeDef tx_ret_val;

	dest_addr = (uint16_t)MsgPtr[0]; // Address already shifted

	// must reconfigure the I2C peripheral before attempting to transmit in master mode
	HAL_I2C_DeInit(&hi2c_ipmbb);
	HAL_I2C_Init(&hi2c_ipmbb);
	HAL_I2CEx_ConfigAnalogFilter(&hi2c_ipmbb, I2C_ANALOGFILTER_ENABLE);
	HAL_I2CEx_ConfigDigitalFilter(&hi2c_ipmbb, 0);

	// set current mode as Master
	i2c_ipmbb_current_state = I2C_MODE_MASTER;

	// begin the transmission
	tx_ret_val = HAL_I2C_Master_Transmit_IT(&hi2c_ipmbb, dest_addr, &MsgPtr[1], (uint16_t) ByteCount -1);
	//tx_ret_val = HAL_I2C_Master_Transmit(&hi2c_ipmbb, dest_addr, &MsgPtr[1], (uint16_t) ByteCount -1, 2000);

	// Wait transmission finish or timeout
	semphr_timeout = xSemaphoreTake ( ipmbb_send_semphr, pdMS_TO_TICKS(100) );

	// return I2C to Slave mode
	i2c_ipmbb_recv_len = 0;
	HAL_I2C_DeInit(&hi2c_ipmbb);
	HAL_I2C_Init(&hi2c_ipmbb);
	HAL_I2CEx_ConfigAnalogFilter(&hi2c_ipmbb, I2C_ANALOGFILTER_ENABLE);
	HAL_I2CEx_ConfigDigitalFilter(&hi2c_ipmbb, 0);
	HAL_I2C_Slave_Receive_IT(&hi2c_ipmbb, &ipmbb_input_buffer[0], IPMB_BUFF_SIZE);
	i2c_ipmbb_current_state = I2C_MODE_SLAVE;

	if ( (tx_ret_val == HAL_OK) && (semphr_timeout != pdFALSE) )
		return IPMB_SEND_DONE;
	else
		return IPMB_SEND_FAIL;
}



int ipmc_ios_ipmba_read(uint8_t *MsgPtr)
{
    int i;
    // Length zero means no message received
    if(i2c_ipmba_recv_len > 0)
    {
        MsgPtr[0] = ipmb_addr << 1;
        for(i=0; (i < i2c_ipmba_recv_len); i++)
            MsgPtr[i+1] = ipmba_input_buffer[i];

        i2c_ipmba_recv_len = 0;
        HAL_I2C_Slave_Receive_IT(&hi2c_ipmba, &ipmba_input_buffer[0], IPMB_BUFF_SIZE);
        ipmc_ios_printf("\n");
        return i+1;
    }
    else
        return 0;
}



int ipmc_ios_ipmbb_read(uint8_t *MsgPtr )
{
    int i;
    // Length zero means no message received
    if(i2c_ipmbb_recv_len > 0)
    {
        MsgPtr[0] = ipmb_addr << 1;
        for(i=0; (i < i2c_ipmbb_recv_len); i++)
            MsgPtr[i+1] = ipmbb_input_buffer[i];

        i2c_ipmbb_recv_len = 0;
        HAL_I2C_Slave_Receive_IT(&hi2c_ipmbb, &ipmbb_input_buffer[0], IPMB_BUFF_SIZE);
        ipmc_ios_printf("\n");
        return i+1;
    }
    else
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





void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	static BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

    if( (hi2c->Instance==I2C1) && (i2c_ipmba_current_state==I2C_MODE_MASTER) )
	{
        xSemaphoreGiveFromISR(ipmba_send_semphr, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
 	else if ( (hi2c->Instance==I2C2) && (i2c_ipmbb_current_state==I2C_MODE_MASTER) )
    {
        xSemaphoreGiveFromISR(ipmbb_send_semphr, &xHigherPriorityTaskWoken);
 		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
 	}
}




void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  /** Error_Handler() function is called when error occurs.
    * 1- When Slave don't acknowledge it's address, Master restarts communication.
    * 2- When Master don't acknowledge the last data transferred, Slave don't care in this example.
    */
	static BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;

	if(hi2c->Instance==I2C1 && i2c_ipmba_current_state == I2C_MODE_SLAVE)
	{
		i2c_ipmba_recv_len = IPMB_BUFF_SIZE - hi2c->XferSize;
		if (i2c_ipmba_recv_len>0)
		{
			xSemaphoreGiveFromISR(ipmb_rec_semphr, &xHigherPriorityTaskWoken);
		}
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
	else if (hi2c->Instance==I2C2 && i2c_ipmbb_current_state == I2C_MODE_SLAVE)
    {
		i2c_ipmbb_recv_len = IPMB_BUFF_SIZE - hi2c->XferSize;
		if (i2c_ipmbb_recv_len>0)
		{
			xSemaphoreGiveFromISR(ipmb_rec_semphr, &xHigherPriorityTaskWoken);
		}
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
	else if ((i2c_ipmba_current_state == I2C_MODE_MASTER) || (i2c_ipmbb_current_state == I2C_MODE_MASTER))
    {
        //ipmc_ios_printf("I2C MASTER SEND FAIL - ISR ERROR CALLED");
    }

}



/*
 * Control the Blue Led
 */
void ipmc_ios_blue_led(int blue_led_state)
{
	if (blue_led_state == 0)
		BLUE_LED_SET_STATE(RESET); // LED OFF
	else
		BLUE_LED_SET_STATE(SET); // LED ON
}



/*
 * Implementation of printf. It uses library developed by Marco Paland (info@paland.com)
 * under MIT license available at github.com/mpaland/printf
 */
void ipmc_ios_printf(const char* format, ...)
{
	va_list args;

	xSemaphoreTake(printf_mutex, portMAX_DELAY);

	va_start( args, format );
	vprintf_( format, args );
	va_end( args );

	xSemaphoreGive(printf_mutex);
}


void _putchar(char character)
{
	HAL_UART_Transmit(&huart4_printout, &character, 1, 1000);
	//outbyte(character);
}

