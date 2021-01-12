


/*
 * This file implements a driver to control all the 90 AMC_IO pins present in
 * DIMM board.
 *
 * This driver takes control of all 6 MCP23S17 devices available in the
 * board, and delivers a simple interface to control all the pins by their
 * number.
 *
 */



#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"

#include "amc_gpios.h"


/*
 * Define how to control EXPANDER_NSS and EXPANDER_RST.
 */
#define EXPANDER_NSS_set_low()  HAL_GPIO_WritePin( EXPANDER_NSS_GPIO_Port, EXPANDER_NSS_Pin,  GPIO_PIN_RESET)
#define EXPANDER_NSS_set_high() HAL_GPIO_WritePin( EXPANDER_NSS_GPIO_Port, EXPANDER_NSS_Pin,  GPIO_PIN_SET  )
#define EXPANDER_RST_set_low()  HAL_GPIO_WritePin( EXPANDER_RST_GPIO_Port, EXPANDER_RST_Pin,  GPIO_PIN_RESET)
#define EXPANDER_RST_set_high() HAL_GPIO_WritePin( EXPANDER_RST_GPIO_Port, EXPANDER_RST_Pin,  GPIO_PIN_SET  )

#define get_AMC_SPI_INT() HAL_GPIO_ReadPin( EXPANDER_INT_GPIO_Port, EXPANDER_INT_Pin )


/*
 * Pin map.
 *
 * It tells how to control an specific AMC_IO signal in the IO expanders
 * (MCP23S17).
 *
 * The array contains all AMC_IO pins organized in sequence (total of 90 pins):
 *   AMC0_IO_0, AMC0_IO_1, AMC0_IO_2 ...
 *                 ... AMC8_IO_7, AMC8_IO_8, AMC8_IO_9
 */
static const struct
{
	uint8_t device; // MCP23S17 hard wired address. From 0 to 5
	uint8_t port;   // MCP23S17 port. 0 = PortA; 1 = PortB
	uint8_t pin;    // MCP23S17 port pin. From 0 to 7

} pin_map[90] = {
// n = 0        1        2        3        4        5        6        7        8        9
    {0,0,0}, {0,0,1}, {0,0,2}, {0,0,3}, {0,0,4}, {0,0,5}, {0,0,6}, {0,0,7}, {0,1,0}, {0,1,1},    // AMC0_IO_n
    {1,1,7}, {1,1,6}, {1,1,5}, {1,1,4}, {1,1,3}, {1,1,2}, {1,1,1}, {1,1,0}, {1,0,7}, {1,0,6},    // AMC1_IO_n
    {0,1,2}, {0,1,3}, {0,1,4}, {0,1,5}, {0,1,6}, {0,1,7}, {2,0,0}, {2,0,1}, {2,0,2}, {2,0,3},    // AMC2_IO_n
    {1,0,5}, {1,0,4}, {1,0,3}, {1,0,2}, {1,0,1}, {1,0,0}, {3,1,7}, {3,1,6}, {3,1,5}, {3,1,4},    // AMC3_IO_n
    {2,0,4}, {2,0,5}, {2,0,6}, {2,0,7}, {2,1,0}, {2,1,1}, {2,1,2}, {2,1,3}, {2,1,4}, {2,1,5},    // AMC4_IO_n
    {3,1,3}, {3,1,2}, {3,1,1}, {3,1,0}, {3,0,7}, {3,0,6}, {3,0,5}, {3,0,4}, {3,0,3}, {3,0,2},    // AMC5_IO_n
    {2,1,6}, {2,1,7}, {4,0,0}, {4,0,1}, {4,0,2}, {4,0,3}, {4,0,4}, {4,0,5}, {4,0,6}, {4,0,7},    // AMC6_IO_n
    {3,0,1}, {3,0,0}, {5,1,7}, {5,1,6}, {5,1,5}, {5,1,4}, {5,1,3}, {5,1,2}, {5,1,1}, {5,1,0},    // AMC7_IO_n
    {4,1,0}, {4,1,1}, {4,1,2}, {4,1,3}, {4,1,4}, {4,1,5}, {4,1,6}, {4,1,7}, {5,0,7}, {5,0,6}     // AMC8_IO_n
};


/*
 * SPI resources
 */

// SPI buffers and global status
static uint8_t spi_data_out[6] __attribute__((section(".sram4")));
static uint8_t spi_data_in[6]  __attribute__((section(".sram4")));
static enum { IDLE, BUSY } spi_status = IDLE;

// SPI Handle
extern SPI_HandleTypeDef hspi4;  //CubeIDE instantiates it in main.c
#define hspi_amc hspi4

// Writes ONE register into an Expander device
#define WRITE_1_REG( DEVICE_ADDR, REG_ADDR, VAL)                              \
	{                                                                         \
		while(spi_status != IDLE) { asm("nop"); }                             \
		spi_status = BUSY;                                                    \
		spi_data_out[0] = 0x40 | (DEVICE_ADDR<<1);                            \
		spi_data_out[1] = REG_ADDR;                                           \
		spi_data_out[2] = VAL;                                                \
		EXPANDER_NSS_set_low();                                               \
		HAL_SPI_TransmitReceive_DMA(&hspi_amc, spi_data_out, spi_data_in, 3); \
	}

// Reads ONE register from an Expander device
#define READ_1_REG( DEVICE_ADDR, REG_ADDR, VAL )                              \
	{                                                                         \
		while(spi_status != IDLE) { asm("nop"); }                             \
		spi_status = BUSY;                                                    \
		spi_data_out[0] = 0x41 | (DEVICE_ADDR<<1);                            \
		spi_data_out[1] = REG_ADDR;                                           \
		spi_data_out[2] = 0  ;                                                \
		EXPANDER_NSS_set_low();                                               \
		HAL_SPI_TransmitReceive_DMA(&hspi_amc, spi_data_out, spi_data_in, 3); \
		while(spi_status != IDLE) { asm("nop"); }                             \
		VAL = spi_data_in[2];                                                 \
	}

// Reads FOUR registers from an Expander device
#define READ_4_REG( DEVICE_ADDR, REG_ADDR, VAL1, VAL2 , VAL3, VAL4 )              \
	{                                                                             \
		while(spi_status != IDLE) { asm("nop"); }                                 \
		spi_status = BUSY;                                                        \
		spi_data_out[0] = 0x41 | (DEVICE_ADDR<<1);                                \
		spi_data_out[1] = REG_ADDR;                                               \
		spi_data_out[2] = spi_data_out[3] = spi_data_out[4] = spi_data_out[5] = 0;\
		EXPANDER_NSS_set_low();                                                   \
		HAL_SPI_TransmitReceive_DMA(&hspi_amc, spi_data_out, spi_data_in, 6);     \
		while(spi_status != IDLE) { asm("nop"); }                                 \
		VAL1 = spi_data_in[2];                                                    \
		VAL2 = spi_data_in[3];                                                    \
		VAL3 = spi_data_in[4];                                                    \
		VAL4 = spi_data_in[5];                                                    \
	}





static amc_int_status_t interrupt_status_buffer[90];

osThreadId_t amc_gpios_pin_interrupt_task_handle;
static osThreadAttr_t amc_gpios_pin_interrupt_task_attributes = {
  .name = "AMC_GPIOS_PIN_INTERRUPT_TASK",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

SemaphoreHandle_t amc_spi_int_falling_edge_semphr = NULL;


/*
 * Concurrency protection resources (for multi-thread)
 */
static SemaphoreHandle_t amc_gpio_mutex = NULL;

#define WAIT_FOR_MUTEX()                                \
	{                                                   \
		while (amc_gpio_mutex == NULL)  { asm("nop"); } \
		xSemaphoreTake( amc_gpio_mutex, portMAX_DELAY );\
	}

#define GIVE_MUTEX() xSemaphoreGive( amc_gpio_mutex )


/*
 * Internal functions
 */
static void    set_expander_register_bit( uint8_t device_addr, uint8_t reg_addr, uint8_t bit_posic, uint8_t bit_value );
static uint8_t get_expander_register_bit( uint8_t device_addr, uint8_t reg_addr, uint8_t bit_posic );
static void    amc_gpios_pin_interrupt_task( void );
static int     auto_test(void);



void amc_gpios_init( void )
{
	int i;
	uint8_t value;

	// Reset expanders
	EXPANDER_RST_set_low();
	HAL_Delay(10);
	EXPANDER_RST_set_high();

	// Configure expanders in "Address Mode".
	// (Since "Address Mode" is not enable after reset, this operation has
	// a broadcast effect)
	// Writes 0x08 in register 0x0A of all devices (dev addr: 0x00)
	WRITE_1_REG(0x00, 0x0A, 0x08);

	// The previous command must be repeated with A2 set to '1'.
	// Even if a device is not in "Address Mode", it is still sensitive to the pin A2.
	// This is a BUG reported in the MCP23S17 Rev. A  Silicon Errata (DS80311A)
	WRITE_1_REG(0x04, 0x0A, 0x08);

	// Execute Auto Test.
	// This test ensures that all expanders are being properly addressed and
	// responsive
	if( auto_test() != 0)
		asm("nop"); //OK
	else
	{
		return; //FAIL
	}

	// Configure INT pins as open-drain
	for( i=0; i<6; i++)
		WRITE_1_REG( i, 0x0A, (0x08|0x04));

	// Check the IOCON configuration
	for( i=0; i<6; i++)
	{
		value = 0;
		READ_1_REG( i, 0x0A, value);
		if (value != (0x08|0x04) )
			return; //FAIL
	}



	// Semaphore to trigger the pin interrupt task  TODO: can be chaged to task notification
	amc_spi_int_falling_edge_semphr = xSemaphoreCreateBinary();

	// TODO: change to FreeRTOS API
	amc_gpios_pin_interrupt_task_handle = osThreadNew((TaskFunction_t)amc_gpios_pin_interrupt_task, NULL, &amc_gpios_pin_interrupt_task_attributes);


	amc_gpio_mutex = xSemaphoreCreateMutex();
}

/*
 * Set the value of one specific bit in one register
 */
static void set_expander_register_bit( uint8_t device_addr, uint8_t reg_addr, uint8_t bit_posic, uint8_t bit_value )
{
	uint8_t val;

	WAIT_FOR_MUTEX();

	// Get current value
	READ_1_REG( device_addr, reg_addr, val);

	val = val &~(0x01<<bit_posic); // The target bit is cleared
	if( bit_value != 0 )
		val |= ( 0x01<<bit_posic );

	// Write new value
	WRITE_1_REG( device_addr, reg_addr, val);

	GIVE_MUTEX();
}

/*
 * get the value of one specific bit in one register
 */
static uint8_t get_expander_register_bit( uint8_t device_addr, uint8_t reg_addr, uint8_t bit_posic )
{
	uint8_t val;

	WAIT_FOR_MUTEX();

	READ_1_REG( device_addr, reg_addr, val);

	GIVE_MUTEX();

	if( ( val & (1<<bit_posic) ) != 0 )
		return 1;
	else
		return 0;
}


void amc_gpios_set_pin_direction( uint8_t amc_pin, amc_dir_t dir )
{
	uint8_t reg_addr;

	// Protection against bad value
	if(amc_pin >= 90) return;

	// Calculate register addr IODIRA or IODIRB. Base addr: 0x00
	reg_addr = 0x00 + pin_map[amc_pin].port;

	set_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin, (uint8_t)dir );
}

amc_dir_t amc_gpios_get_pin_direction( uint8_t amc_pin )
{
	uint8_t reg_addr;

	// Protection against bad value
	if(amc_pin >= 90) return 0;

	// Calculate register addr IODIRA or IODIRB. Base addr: 0x00
	reg_addr = 0x00 + pin_map[amc_pin].port;

	return get_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin );
}

void amc_gpios_write_pin( uint8_t amc_pin, uint8_t pin_value )
{
	uint8_t reg_addr;

	// Protection against bad value
	if(amc_pin >= 90) return;

	// Calculate register addr GPIOA or GPIOB. Base addr: 0x12
	reg_addr = 0x12 + pin_map[amc_pin].port;

	set_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin, pin_value );
}

uint8_t amc_gpios_read_pin( uint8_t amc_pin )
{
	uint8_t reg_addr;

	// Protection against bad value
	if(amc_pin >= 90) return 0;

	// Calculate register addr GPIOA or GPIOB. Base addr: 0x12
	reg_addr = 0x12 + pin_map[amc_pin].port;

	return get_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin );
}

void amc_gpios_set_pin_pullup( uint8_t amc_pin, amc_pullup_t pullup_state )
{
	uint8_t reg_addr;

	// Protection against bad value
	if(amc_pin >= 90) return;

	// Calculate register addr GPPUA or GPPUB. Base addr: 0x0C
	reg_addr = 0x0C + pin_map[amc_pin].port;

	set_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin, pullup_state );
}

amc_pullup_t amc_gpios_get_pin_pullup( uint8_t amc_pin )
{
	uint8_t reg_addr;

	// Protection against bad value
	if(amc_pin >= 90) return 0;

	// Calculate register addr GPPUA or GPPUB. Base addr: 0x0C
	reg_addr = 0x0C + pin_map[amc_pin].port;

	return get_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin );
}

void amc_gpios_set_pin_interrupt( uint8_t amc_pin, amc_int_mode_t mode )
{
	uint8_t reg_addr;
	uint8_t gpinten_value;
	uint8_t intcon_value;
	uint8_t defval_value;


	// Protection against bad value
	if(amc_pin >= 90) return ;

	switch(mode)
	{
	case AMC_INT_BOTH_EDGES:
		gpinten_value = 1;
		intcon_value  = 0;
		defval_value  = 0;
		break;

	//case AMC_INT_RISING_EDGE:   To be developed
	//	gpinten_value = 1;
	//	intcon_value  = 1;
	//	defval_value  = 0;
	//	break;

	//case AMC_INT_FALLING_EDGE:  To be developed
	//	gpinten_value = 1;
	//	intcon_value  = 1;
	//	defval_value  = 1;
	//	break;

	case AMC_INT_OFF:
	default:
		gpinten_value = 0;
		intcon_value  = 0;
		defval_value  = 0;
		break;
	}

	// Write registers
	reg_addr = 0x06 + pin_map[amc_pin].port;
	set_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin, defval_value );
	reg_addr = 0x08 + pin_map[amc_pin].port;
	set_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin, intcon_value );
	reg_addr = 0x04 + pin_map[amc_pin].port;
	set_expander_register_bit( pin_map[amc_pin].device, reg_addr, pin_map[amc_pin].pin, gpinten_value );
}

/*
 * Task dedicated to solve interrupts generated by the MCP23S17 devices
 *
 * This task basically responds to a falling edge in the AMC_SPI_INT.
 *
 */
static void amc_gpios_pin_interrupt_task( void )
{
	int ctr;
	uint8_t intf_value[6][2];
	uint8_t intcap_value[6][2];
	uint8_t device, port, pin;

	for(;;)
	{
		// Wait for a falling edge on AMC_SPI_INT signal
		xSemaphoreTake( amc_spi_int_falling_edge_semphr, portMAX_DELAY );

		do
		{
			// Read the "dafault" values and the captured values from all devices
			WAIT_FOR_MUTEX();
			for( ctr=0; ctr<6; ctr++ )
			{
				READ_4_REG( ctr, 0x0E, intf_value[ctr][0], intf_value[ctr][1], intcap_value[ctr][0], intcap_value[ctr][1] );
			}
			GIVE_MUTEX();

			// Evaluate the interrupt status for all pins
			for( ctr=0; ctr<90; ctr++ )
			{
				device = pin_map[ctr].device;
				port = pin_map[ctr].port;
				pin = pin_map[ctr].pin;
				if( intf_value[device][port] & (1<<pin) )
				{
					if( intcap_value[device][port] & (1<<pin) )
						interrupt_status_buffer[ctr] = AMC_INT_STATUS_RISED;
					else
						interrupt_status_buffer[ctr] = AMC_INT_STATUS_FALLED;
				}
				else
					interrupt_status_buffer[ctr] = AMC_INT_STATUS_NONE;

			}

			// Call the callback
			amc_gpios_pin_interrupt_callback( interrupt_status_buffer );

		} while(get_AMC_SPI_INT() == 0);
	}
}


/*
 * SPI interrupt
 *
 * This functions must be called from the SPI ISR after transaction is
 * completed.
 */
void amc_gpios_spi_interrupt( void )
{
	EXPANDER_NSS_set_high();
	spi_status = IDLE;
}

/*
 * A very nice pseudo random sequence generator
 */
static uint16_t random_word( void )
{
	static uint8_t reg= 0x55;

	// Maximal LFSR
	// Taps: 8 6 5 4; Polynomial: x^8 + x^6 + x^5 + x^4 + 1
	// Length: 256
	reg = (reg<<1) | ( ( (reg>>7)^(reg>>5)^(reg>>4)^(reg>>3) )&0x01 );

	return reg;
}

/*
 * Autotest function.
 *
 * It writes random data across the devices and check them. It ensures correct
 * communication between STM32 and MCP23S17 devices.
 */
static int auto_test( void )
{
	int test_ctr, dev_ctr;
	uint8_t test_value[6][2];
	uint8_t rd_value;

	// Executes 32 tests in each device
	for(test_ctr = 0; test_ctr<32; test_ctr++)
	{
		// Pick random numbers and write them into the devices (Uses OLAT registers)
		for(dev_ctr = 0; dev_ctr<6; dev_ctr++)
		{
			test_value[dev_ctr][0] = random_word();
			test_value[dev_ctr][1] = random_word();
			WRITE_1_REG( dev_ctr, 0x14, test_value[dev_ctr][0]);
			WRITE_1_REG( dev_ctr, 0x15, test_value[dev_ctr][1]);
		}

		// Check
		for(dev_ctr = 0; dev_ctr<6; dev_ctr++)
		{
			READ_1_REG( dev_ctr, 0x14, rd_value );
			if( rd_value != test_value[dev_ctr][0] ) return 0; // Test fail
			READ_1_REG( dev_ctr, 0x15, rd_value );
			if( rd_value != test_value[dev_ctr][1] ) return 0; // Test fail
		}
	}

	// Set all values back to ZERO (Default)
	for(dev_ctr = 0; dev_ctr<6; dev_ctr++)
	{
		WRITE_1_REG( dev_ctr, 0x14, 0x00 );
		WRITE_1_REG( dev_ctr, 0x15, 0x00 );
	}

	return 1; // Auto Test OK
}





