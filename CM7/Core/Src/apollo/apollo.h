#ifndef __APOLLO_H_
#define __APOLLO_H_

#include "../dimm_gpios.h"

// add a layer with an extra level of indirection to allow macro expansion in the arguments of the macro..
// uhg...
// https://stackoverflow.com/questions/1489932/how-to-concatenate-twice-with-the-c-preprocessor-and-expand-a-macro-as-in-arg
// otherwise the preprocessor just pastes in e.g. APOLLO_FPGA_DONE instead of USR_IO_2
#define GPIO_CONFIGURE_PIN_EXPAND(NAME, MODE, PULL) GPIO_CONFIGURE_PIN(NAME, MODE, PULL)
#define GPIO_CONFIGURE_PIN_EXPAND(NAME, MODE, PULL) GPIO_CONFIGURE_PIN(NAME, MODE, PULL)
#define GPIO_GET_STATE_EXPAND(NAME) GPIO_GET_STATE(NAME)
#define GPIO_SET_STATE_EXPAND(STATE, NAME) GPIO_SET_STATE_VAL(STATE, NAME)

// normal set state function is expecting SET or RESET, but does not take a normal value e.g. 0 1
// create our own flavor of the macro to do this
#define GPIO_SET_STATE_VAL(val, NAME)                       \
  HAL_GPIO_WritePin( NAME ## _GPIO_Port, NAME ## _Pin, val)

#define APOLLO_REV1  0x3
#define APOLLO_REV2  0x2
#define APOLLO_REV2A 0x1

#define APOLLO_HANDLE_OPEN 1

#define APOLLO_BOOT_SD   3
#define APOLLO_BOOT_QSPI 2
#define APOLLO_BOOT_NAND 1
#define APOLLO_BOOT_JTAG 0

#define APOLLO_UART_ZYNQ 0
#define APOLLO_UART_DISCONNECT 1
#define APOLLO_UART_MEZZ1 2
#define APOLLO_UART_MEZZ2 3

#define APOLLO_FPGA_DONE        USR_IO_2
#define APOLLO_UART_ADR_0       USR_IO_5
#define APOLLO_UART_ADR_1       USR_IO_6
#define APOLLO_BOOT_MODE_0      USR_IO_7
#define APOLLO_BOOT_MODE_1      USR_IO_8
#define APOLLO_SENSE_RESET_N    USR_IO_9
#define APOLLO_MEZZ1_EN         USR_IO_11
#define APOLLO_MEZZ2_EN         USR_IO_10
#define APOLLO_M24512_WE_N      USR_IO_12
#define APOLLO_ETH_SW_PWR_GOOD  USR_IO_13
#define APOLLO_ZYNQ_EN          USR_IO_3
#define APOLLO_ZYNQ_UP          USR_IO_14
#define APOLLO_NO_SHELF         USR_IO_15
#define APOLLO_ESM_RESET_N      USR_IO_16
#define APOLLO_ZYNQ_0           USR_IO_18
#define APOLLO_ZYNQ_1           USR_IO_20
#define APOLLO_ZYNQ_2           USR_IO_22
#define APOLLO_ZYNQ_3           USR_IO_24
#define APOLLO_ZYNQ_4           USR_IO_26
#define APOLLO_ZYNQ_5           USR_IO_28
#define APOLLO_REVID_0          USR_IO_30
#define APOLLO_REVID_1          USR_IO_32
#define APOLLO_JTAG_CHAIN_SEL_0 USR_IO_4
#define APOLLO_JTAG_CHAIN_SEL_1 USR_IO_33
#define APOLLO_JTAG_CHAIN_SEL_2 USR_IO_34
#define APOLLO_GPIO_0           USR_IO_17
#define APOLLO_GPIO_1           USR_IO_19
#define APOLLO_GPIO_2           USR_IO_21
#define APOLLO_GPIO_3           USR_IO_23
#define APOLLO_GPIO_4           USR_IO_25
#define APOLLO_GPIO_5           USR_IO_27
#define APOLLO_GPIO_6           USR_IO_29
#define APOLLO_GPIO_7           USR_IO_31

uint8_t apollo_get_ipmc_startup_done ();
uint8_t apollo_get_ipmc_startup_started ();
uint8_t apollo_get_ipmc_abnormal_shutdown ();

uint8_t apollo_get_esm_pwr_good ();
uint8_t apollo_get_fpga_done ();
uint8_t apollo_get_noshelf ();
uint8_t apollo_get_revision ();
uint8_t apollo_get_zynq_up ();
uint8_t apollo_get_zynq_boot_mode ();

void apollo_set_cm_enable (uint8_t enable);
void apollo_set_eeprom_we_n (uint8_t state);
void apollo_set_esm_reset_n (uint8_t state);
void apollo_set_jtag_chain_sel (uint8_t sel);
void apollo_set_sense_reset_n (uint8_t state);
void apollo_set_uart_adr (uint8_t adr);
void apollo_set_zynq_boot_mode (uint8_t mode);
void apollo_set_zynq_en (uint8_t state);

void apollo_esm_reset();
void apollo_init_gpios ();
void apollo_init_bootmode ();
void apollo_powerdown_sequence ();
void apollo_powerup_sequence ();

char* get_apollo_status () ;

uint8_t apollo_get_handle_open ();

#define APOLLO_STATUS_PU_INIT_IO  0
#define APOLLO_STATUS_PU_SET_BOOT_MODE  1
#define APOLLO_STATUS_PU_SET_UART_ADR  2
#define APOLLO_STATUS_PU_SET_CHAIN_SEL  3
#define APOLLO_STATUS_PU_SET_EEPROM_WE  4
#define APOLLO_STATUS_PU_RESET_I2C_MUX  5
#define APOLLO_STATUS_PU_SET_12V_EN  6
#define APOLLO_STATUS_PU_SET_ZYNQ_EN  7
#define APOLLO_STATUS_PU_TIMEOUT_FPGA_DONE  8
#define APOLLO_STATUS_PU_WAIT_FPGA_DONE  9
#define APOLLO_STATUS_PU_TIMEOUT_ZYNQ_DONE  10
#define APOLLO_STATUS_PU_DONE  11
#define APOLLO_STATUS_PU_WAIT_ZYNQ_DONE  12
#define APOLLO_STATUS_PD_WAIT_ZYNQ_OFF  13
#define APOLLO_STATUS_PD_TIMEOUT_ZYNQ_OFF  14
#define APOLLO_STATUS_PD_12V_PD  15
#define APOLLO_STATUS_PD_DONE  16

#define APOLLO_ERR_TIMEOUT_ZYNQ_FPGA 1 
#define APOLLO_ERR_TIMEOUT_ZYNQ_CPU 2 
#define APOLLO_ERR_TIMEOUT_ZYNQ_SHUTDOWN 3 
#define APOLLO_ERR_OPEN_HANDLE 4

#endif // __APOLLO_H_
