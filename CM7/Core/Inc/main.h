/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define I2C_BB_SCL_Pin GPIO_PIN_6
#define I2C_BB_SCL_GPIO_Port GPIOI
#define I2C_BB_SDA_Pin GPIO_PIN_5
#define I2C_BB_SDA_GPIO_Port GPIOI
#define IPM_0_Pin GPIO_PIN_4
#define IPM_0_GPIO_Port GPIOI
#define USR_IO_6_Pin GPIO_PIN_5
#define USR_IO_6_GPIO_Port GPIOB
#define IPM_1_Pin GPIO_PIN_7
#define IPM_1_GPIO_Port GPIOI
#define USR_IO_0_Pin GPIO_PIN_1
#define USR_IO_0_GPIO_Port GPIOE
#define USR_IO_1_Pin GPIO_PIN_0
#define USR_IO_1_GPIO_Port GPIOE
#define EXPANDER_INT_Pin GPIO_PIN_3
#define EXPANDER_INT_GPIO_Port GPIOI
#define EXPANDER_INT_EXTI_IRQn EXTI3_IRQn
#define EXPANDER_NSS_Pin GPIO_PIN_4
#define EXPANDER_NSS_GPIO_Port GPIOE
#define EXPANDER_RST_Pin GPIO_PIN_3
#define EXPANDER_RST_GPIO_Port GPIOE
#define IPM_14_Pin GPIO_PIN_14
#define IPM_14_GPIO_Port GPIOG
#define USR_IO_34_Pin GPIO_PIN_10
#define USR_IO_34_GPIO_Port GPIOA
#define ETH_INT_N_Pin GPIO_PIN_13
#define ETH_INT_N_GPIO_Port GPIOC
#define ETH_RST_N_Pin GPIO_PIN_8
#define ETH_RST_N_GPIO_Port GPIOI
#define IPM_13_Pin GPIO_PIN_7
#define IPM_13_GPIO_Port GPIOC
#define IPM_8_Pin GPIO_PIN_6
#define IPM_8_GPIO_Port GPIOC
#define USR_IO_7_Pin GPIO_PIN_8
#define USR_IO_7_GPIO_Port GPIOG
#define IPM_2_Pin GPIO_PIN_2
#define IPM_2_GPIO_Port GPIOF
#define IPM_7_Pin GPIO_PIN_5
#define IPM_7_GPIO_Port GPIOG
#define IPM_3_Pin GPIO_PIN_12
#define IPM_3_GPIO_Port GPIOI
#define IPM_4_Pin GPIO_PIN_13
#define IPM_4_GPIO_Port GPIOI
#define IPM_5_Pin GPIO_PIN_14
#define IPM_5_GPIO_Port GPIOI
#define IPM_6_Pin GPIO_PIN_3
#define IPM_6_GPIO_Port GPIOF
#define IPMB_B_EN_Pin GPIO_PIN_4
#define IPMB_B_EN_GPIO_Port GPIOG
#define IPMB_B_RDY_Pin GPIO_PIN_3
#define IPMB_B_RDY_GPIO_Port GPIOG
#define IPMB_A_EN_Pin GPIO_PIN_2
#define IPMB_A_EN_GPIO_Port GPIOG
#define IPMB_A_RDY_Pin GPIO_PIN_2
#define IPMB_A_RDY_GPIO_Port GPIOK
#define IPM_12_Pin GPIO_PIN_5
#define IPM_12_GPIO_Port GPIOF
#define IPM_9_Pin GPIO_PIN_4
#define IPM_9_GPIO_Port GPIOF
#define HW_5_Pin GPIO_PIN_0
#define HW_5_GPIO_Port GPIOK
#define HW_7_Pin GPIO_PIN_1
#define HW_7_GPIO_Port GPIOK
#define HW_3_Pin GPIO_PIN_11
#define HW_3_GPIO_Port GPIOJ
#define USR_IO_2_Pin GPIO_PIN_0
#define USR_IO_2_GPIO_Port GPIOC
#define HW_1_Pin GPIO_PIN_10
#define HW_1_GPIO_Port GPIOJ
#define HW_6_Pin GPIO_PIN_9
#define HW_6_GPIO_Port GPIOJ
#define IPM_10_Pin GPIO_PIN_0
#define IPM_10_GPIO_Port GPIOA
#define IPM_15_Pin GPIO_PIN_0
#define IPM_15_GPIO_Port GPIOJ
#define IPM_11_Pin GPIO_PIN_10
#define IPM_11_GPIO_Port GPIOE
#define HW_4_Pin GPIO_PIN_8
#define HW_4_GPIO_Port GPIOJ
#define MASTER_TDI_Pin GPIO_PIN_7
#define MASTER_TDI_GPIO_Port GPIOJ
#define MASTER_TMS_Pin GPIO_PIN_6
#define MASTER_TMS_GPIO_Port GPIOJ
#define USR_IO_3_Pin GPIO_PIN_4
#define USR_IO_3_GPIO_Port GPIOH
#define ETH_TX_ER_Pin GPIO_PIN_5
#define ETH_TX_ER_GPIO_Port GPIOH
#define USR_IO_10_Pin GPIO_PIN_15
#define USR_IO_10_GPIO_Port GPIOI
#define USR_IO_11_Pin GPIO_PIN_1
#define USR_IO_11_GPIO_Port GPIOJ
#define ALARM_A_Pin GPIO_PIN_13
#define ALARM_A_GPIO_Port GPIOF
#define USR_IO_23_Pin GPIO_PIN_9
#define USR_IO_23_GPIO_Port GPIOE
#define USR_IO_25_Pin GPIO_PIN_11
#define USR_IO_25_GPIO_Port GPIOE
#define USR_IO_27_Pin GPIO_PIN_10
#define USR_IO_27_GPIO_Port GPIOB
#define HW_2_Pin GPIO_PIN_10
#define HW_2_GPIO_Port GPIOH
#define MASTER_TCK_Pin GPIO_PIN_11
#define MASTER_TCK_GPIO_Port GPIOH
#define ALARM_B_Pin GPIO_PIN_15
#define ALARM_B_GPIO_Port GPIOD
#define MASTER_TDO_Pin GPIO_PIN_14
#define MASTER_TDO_GPIO_Port GPIOD
#define USR_IO_4_Pin GPIO_PIN_6
#define USR_IO_4_GPIO_Port GPIOA
#define USR_IO_14_Pin GPIO_PIN_2
#define USR_IO_14_GPIO_Port GPIOB
#define USR_IO_18_Pin GPIO_PIN_12
#define USR_IO_18_GPIO_Port GPIOF
#define USR_IO_28_Pin GPIO_PIN_12
#define USR_IO_28_GPIO_Port GPIOE
#define USR_IO_29_Pin GPIO_PIN_15
#define USR_IO_29_GPIO_Port GPIOE
#define USR_IO_31_Pin GPIO_PIN_5
#define USR_IO_31_GPIO_Port GPIOJ
#define HW_0_Pin GPIO_PIN_9
#define HW_0_GPIO_Port GPIOH
#define PWR_GOOD_A_Pin GPIO_PIN_12
#define PWR_GOOD_A_GPIO_Port GPIOH
#define EN_12V_Pin GPIO_PIN_11
#define EN_12V_GPIO_Port GPIOD
#define FP_LED_2_Pin GPIO_PIN_12
#define FP_LED_2_GPIO_Port GPIOD
#define MASTER_TRST_Pin GPIO_PIN_13
#define MASTER_TRST_GPIO_Port GPIOD
#define USR_IO_5_Pin GPIO_PIN_5
#define USR_IO_5_GPIO_Port GPIOA
#define USR_IO_15_Pin GPIO_PIN_2
#define USR_IO_15_GPIO_Port GPIOJ
#define USR_IO_19_Pin GPIO_PIN_11
#define USR_IO_19_GPIO_Port GPIOF
#define USR_IO_21_Pin GPIO_PIN_0
#define USR_IO_21_GPIO_Port GPIOG
#define USR_IO_20_Pin GPIO_PIN_8
#define USR_IO_20_GPIO_Port GPIOE
#define USR_IO_24_Pin GPIO_PIN_13
#define USR_IO_24_GPIO_Port GPIOE
#define USR_IO_32_Pin GPIO_PIN_8
#define USR_IO_32_GPIO_Port GPIOH
#define HANDLE_Pin GPIO_PIN_12
#define HANDLE_GPIO_Port GPIOB
#define PYLD_RESET_Pin GPIO_PIN_15
#define PYLD_RESET_GPIO_Port GPIOB
#define FP_LED_0_Pin GPIO_PIN_10
#define FP_LED_0_GPIO_Port GPIOD
#define FP_LED_1_Pin GPIO_PIN_9
#define FP_LED_1_GPIO_Port GPIOD
#define USR_IO_8_Pin GPIO_PIN_3
#define USR_IO_8_GPIO_Port GPIOA
#define USR_IO_9_Pin GPIO_PIN_4
#define USR_IO_9_GPIO_Port GPIOA
#define USR_IO_12_Pin GPIO_PIN_0
#define USR_IO_12_GPIO_Port GPIOB
#define USR_IO_13_Pin GPIO_PIN_3
#define USR_IO_13_GPIO_Port GPIOJ
#define USR_IO_16_Pin GPIO_PIN_4
#define USR_IO_16_GPIO_Port GPIOJ
#define USR_IO_17_Pin GPIO_PIN_1
#define USR_IO_17_GPIO_Port GPIOG
#define USR_IO_22_Pin GPIO_PIN_7
#define USR_IO_22_GPIO_Port GPIOE
#define USR_IO_26_Pin GPIO_PIN_14
#define USR_IO_26_GPIO_Port GPIOE
#define USR_IO_30_Pin GPIO_PIN_7
#define USR_IO_30_GPIO_Port GPIOH
#define PWR_GOOD_B_Pin GPIO_PIN_13
#define PWR_GOOD_B_GPIO_Port GPIOB
#define USR_IO_33_Pin GPIO_PIN_14
#define USR_IO_33_GPIO_Port GPIOB
#define FP_LED_BLUE_Pin GPIO_PIN_8
#define FP_LED_BLUE_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
