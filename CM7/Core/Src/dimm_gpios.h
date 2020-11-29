#ifndef GPIO_MAP_H
#define GPIO_MAP_H



/*
 * Set the output state of a "named" GPIO in the board
 *
 * STATE argument :
 *     SET or RESET
 *
 * NAME argument:
 *     The name of GPIO as it is on *.ioc file
 *     example: FP_LED_BLUE (see main.h for complete list)
 */
#define GPIO_SET_STATE(STATE, NAME) \
                                    \
	HAL_GPIO_WritePin( NAME ## _GPIO_Port, NAME ## _Pin, GPIO_PIN_ ## STATE )



/*
 * Read the input state of a "named" GPIO in the board
 *
 * NAME argument:
 *     The name of GPIO as it is on *.ioc file
 *     example: HANDLE (see main.h for complete list)
 *
 * Return value (follows HAL_GPIO specs):
 *     GPIO_PIN_SET
 *     GPIO_PIN_RESET
 *     (also can be interpreted as TRUE or FALSE)
 *
 */
#define GPIO_GET_STATE(NAME) \
                             \
	HAL_GPIO_ReadPin( NAME ## _GPIO_Port, NAME ## _Pin )



/*
 * Set the configuration of IPM and USER_IO pins
 *
 * NUM argument must be:
 *     IPM number, from 0 to 15
 *     USER_IO number, from 0 to 34
 *
 * MODE follows HAL_GPIO specs
 *
 * PULL follows HAL_GPIO specs
 *
 */
#define GPIO_CONFIGURE_PIN(NAME, MODE, PULL) \
	{                                                         \
		GPIO_InitTypeDef GPIO_InitStruct = {0};               \
		GPIO_InitStruct.Pin = NAME ## _Pin;                   \
		GPIO_InitStruct.Mode = MODE;                          \
		GPIO_InitStruct.Pull = PULL;                          \
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;          \
		HAL_GPIO_Init( NAME ## _GPIO_Port, &GPIO_InitStruct); \
	}



/*
 * Set specific Outputs
 *
 * STATE argument :
 *     SET or RESET
 */
#define BLUE_LED_SET_STATE(STATE)         GPIO_SET_STATE( STATE, FP_LED_BLUE )
#define LED_0_SET_STATE(STATE)            GPIO_SET_STATE( STATE, FP_LED_0    )
#define LED_1_SET_STATE(STATE)            GPIO_SET_STATE( STATE, FP_LED_1    )
#define LED_2_SET_STATE(STATE)            GPIO_SET_STATE( STATE, FP_LED_2    )
#define EN_12V_SET_STATE(STATE)           GPIO_SET_STATE( STATE, EN_12V      )
#define PAYLOAD_RESET_SET_STATE(STATE)    GPIO_SET_STATE( STATE, PYLD_RESET  )



/*
 * Read specific Inputs
 *
 * Return value (follows HAL_GPIO specs):
 *     GPIO_PIN_SET
 *     GPIO_PIN_RESET
 *     (also can be interpreted as TRUE or FALSE)
 */
#define IPMB_A_RDY_GET_STATE()     GPIO_GET_STATE( IPMB_A_RDY )
#define IPMB_B_RDY_GET_STATE()     GPIO_GET_STATE( IPMB_B_RDY )
#define HANDLE_GET_STATE()         GPIO_GET_STATE( HANDLE     )
#define HW_0_GET_STATE()           GPIO_GET_STATE( HW_0       )
#define HW_1_GET_STATE()           GPIO_GET_STATE( HW_1       )
#define HW_2_GET_STATE()           GPIO_GET_STATE( HW_2       )
#define HW_3_GET_STATE()           GPIO_GET_STATE( HW_3       )
#define HW_4_GET_STATE()           GPIO_GET_STATE( HW_4       )
#define HW_5_GET_STATE()           GPIO_GET_STATE( HW_5       )
#define HW_6_GET_STATE()           GPIO_GET_STATE( HW_6       )
#define HW_7_GET_STATE()           GPIO_GET_STATE( HW_7       )



#endif //GPIO_MAP_H
