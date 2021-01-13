/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stream_buffer.h"
#include "dimm_gpios.h"
#include "amc_gpios.h"
#include "printf.h"
#include "ipmc_tasks.h"
#include "ipmc_ios.h"
#include "fru_state_machine.h"
#include "ipmb_0.h"
#include "amc_gpios.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi4;
DMA_HandleTypeDef hdma_spi4_tx;
DMA_HandleTypeDef hdma_spi4_rx;

UART_HandleTypeDef huart4;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* USER CODE BEGIN PV */

static char uart4_input_char;
StreamBufferHandle_t uart4_input_stram;
osThreadId_t keyboardTaskHandle;
const osThreadAttr_t keyboardTask_attributes = {
  .name = "KeyboardTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};


osThreadId_t ipmb_0_msg_receiver_task_handle;
const osThreadAttr_t ipmb_0_msg_receiver_task_attributes = {
  .name = "MNG",
  .priority = (osPriority_t) osPriorityNormal3,
  .stack_size = 512 * 4
};

osThreadId_t ipmb_0_msg_sender_task_handle;
const osThreadAttr_t ipmb_0_msg_sender_task_attributes = {
  .name = "MNGO",
  .priority = (osPriority_t) osPriorityNormal1,
  .stack_size = 512 * 4
};

osThreadId_t fru_state_machine_task_handle;
const osThreadAttr_t fru_state_machine_task_attributes = {
  .name = "SMM",
  .priority = (osPriority_t) osPriorityNormal1,
  .stack_size = 512 * 4
};

osThreadId_t ipmi_income_requests_manager_task_handle;
const osThreadAttr_t ipmi_income_requests_manager_task_attributes = {
  .name = "IPMI_MSG_MGMT",
  .priority = (osPriority_t) osPriorityNormal2,
  .stack_size = 512 * 4
};

osThreadId_t ipmc_handle_switch_task_handle;
const osThreadAttr_t ipmc_handle_switch_task_attributes = {
  .name = "IPMC_HANDLE_TRANS",
  .priority = (osPriority_t) osPriorityNormal2,
  .stack_size = 128 * 4
};

osThreadId_t ipmc_blue_led_blink_task_handle;
const osThreadAttr_t ipmc_blue_led_blink_task_attributes = {
  .name = "BLUE_LED",
  .priority = (osPriority_t) osPriorityNormal2,
  .stack_size = 128 * 4
};

//osThreadId_t keyboard_input_task_handle;
//const osThreadAttr_t keyboard_input_task_attributes = {
//  .name = "KEYBOARD",
//  .priority = (osPriority_t) osPriorityNormal,
//  .stack_size = 128 * 4
//};


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_UART4_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_SPI4_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
void KeyboardTask(void *argument);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
/* USER CODE BEGIN Boot_Mode_Sequence_0 */
  int32_t timeout;
/* USER CODE END Boot_Mode_Sequence_0 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */
  /* Wait until CPU2 boots and enters in stop mode or timeout*/
  timeout = 0xFFFF;
  while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) && (timeout-- > 0));
  if ( timeout < 0 )
  {
  //Error_Handler();  Unnecessary Error Handler. Just go ahead.
  }
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
/* USER CODE BEGIN Boot_Mode_Sequence_2 */
/* When system initialization is finished, Cortex-M7 will release Cortex-M4 by means of
HSEM notification */
/*HW semaphore Clock enable*/
__HAL_RCC_HSEM_CLK_ENABLE();
/*Take HSEM */
HAL_HSEM_FastTake(HSEM_ID_0);
/*Release HSEM in order to notify the CPU2(CM4)*/
HAL_HSEM_Release(HSEM_ID_0,0);
/* wait until CPU2 wakes up from stop mode */
timeout = 0xFFFF;
while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0));
if ( timeout < 0 )
{
Error_Handler();
}
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_UART4_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_SPI4_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  keyboardTaskHandle = osThreadNew(KeyboardTask, NULL, &keyboardTask_attributes);

  ipmb_0_msg_receiver_task_handle = osThreadNew(ipmb_0_msg_receiver_task, NULL, &ipmb_0_msg_receiver_task_attributes);
  ipmb_0_msg_sender_task_handle = osThreadNew(ipmb_0_msg_sender_task, NULL, &ipmb_0_msg_sender_task_attributes);
  fru_state_machine_task_handle = osThreadNew(fru_state_machine_task, NULL, &fru_state_machine_task_attributes);
  ipmi_income_requests_manager_task_handle = osThreadNew(ipmi_income_requests_manager_task, NULL, &ipmi_income_requests_manager_task_attributes);
  ipmc_handle_switch_task_handle = osThreadNew(ipmc_handle_switch_task, NULL, &ipmc_handle_switch_task_attributes);
  ipmc_blue_led_blink_task_handle = osThreadNew(ipmc_blue_led_blink_task, NULL, &ipmc_blue_led_blink_task_attributes);
  //keyboard_input_task_handle = osThreadNew(KeyboardInput_task, NULL, &keyboard_input_task_attributes);

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_SMPS_1V8_SUPPLIES_LDO);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 32;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_SPI4
                              |RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_I2C1;
  PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x10C0ECFF;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_ENABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x10C0ECFF;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_ENABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI4_Init(void)
{

  /* USER CODE BEGIN SPI4_Init 0 */

  /* USER CODE END SPI4_Init 0 */

  /* USER CODE BEGIN SPI4_Init 1 */

  /* USER CODE END SPI4_Init 1 */
  /* SPI4 parameter configuration*/
  hspi4.Instance = SPI4;
  hspi4.Init.Mode = SPI_MODE_MASTER;
  hspi4.Init.Direction = SPI_DIRECTION_2LINES;
  hspi4.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi4.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi4.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi4.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi4.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi4.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi4.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi4.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi4.Init.CRCPolynomial = 0x0;
  hspi4.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi4.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi4.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi4.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi4.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi4.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi4.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi4.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi4.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi4.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI4_Init 2 */

  /* USER CODE END SPI4_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(EXPANDER_RST_GPIO_Port, EXPANDER_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ETH_RST_N_GPIO_Port, ETH_RST_N_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, IPMB_B_EN_Pin|IPMB_A_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ETH_TX_ER_GPIO_Port, ETH_TX_ER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, EN_12V_Pin|FP_LED_2_Pin|FP_LED_0_Pin|FP_LED_1_Pin
                          |FP_LED_BLUE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PYLD_RESET_GPIO_Port, PYLD_RESET_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : I2C_BB_SCL_Pin I2C_BB_SDA_Pin */
  GPIO_InitStruct.Pin = I2C_BB_SCL_Pin|I2C_BB_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pins : IPM_0_Pin IPM_1_Pin IPM_3_Pin IPM_4_Pin
                           IPM_5_Pin USR_IO_10_Pin */
  GPIO_InitStruct.Pin = IPM_0_Pin|IPM_1_Pin|IPM_3_Pin|IPM_4_Pin
                          |IPM_5_Pin|USR_IO_10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pins : USR_IO_6_Pin USR_IO_27_Pin USR_IO_14_Pin HANDLE_Pin
                           USR_IO_12_Pin PWR_GOOD_B_Pin USR_IO_33_Pin */
  GPIO_InitStruct.Pin = USR_IO_6_Pin|USR_IO_27_Pin|USR_IO_14_Pin|HANDLE_Pin
                          |USR_IO_12_Pin|PWR_GOOD_B_Pin|USR_IO_33_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : USR_IO_0_Pin USR_IO_1_Pin IPM_11_Pin USR_IO_23_Pin
                           USR_IO_25_Pin USR_IO_28_Pin USR_IO_29_Pin USR_IO_20_Pin
                           USR_IO_24_Pin USR_IO_22_Pin USR_IO_26_Pin */
  GPIO_InitStruct.Pin = USR_IO_0_Pin|USR_IO_1_Pin|IPM_11_Pin|USR_IO_23_Pin
                          |USR_IO_25_Pin|USR_IO_28_Pin|USR_IO_29_Pin|USR_IO_20_Pin
                          |USR_IO_24_Pin|USR_IO_22_Pin|USR_IO_26_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : EXPANDER_INT_Pin */
  GPIO_InitStruct.Pin = EXPANDER_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(EXPANDER_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : EXPANDER_RST_Pin */
  GPIO_InitStruct.Pin = EXPANDER_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(EXPANDER_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IPM_14_Pin USR_IO_7_Pin IPM_7_Pin USR_IO_21_Pin
                           USR_IO_17_Pin */
  GPIO_InitStruct.Pin = IPM_14_Pin|USR_IO_7_Pin|IPM_7_Pin|USR_IO_21_Pin
                          |USR_IO_17_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : USR_IO_34_Pin IPM_10_Pin USR_IO_4_Pin USR_IO_5_Pin
                           USR_IO_8_Pin USR_IO_9_Pin */
  GPIO_InitStruct.Pin = USR_IO_34_Pin|IPM_10_Pin|USR_IO_4_Pin|USR_IO_5_Pin
                          |USR_IO_8_Pin|USR_IO_9_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ETH_INT_N_Pin */
  GPIO_InitStruct.Pin = ETH_INT_N_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(ETH_INT_N_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ETH_RST_N_Pin */
  GPIO_InitStruct.Pin = ETH_RST_N_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ETH_RST_N_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IPM_13_Pin IPM_8_Pin USR_IO_2_Pin */
  GPIO_InitStruct.Pin = IPM_13_Pin|IPM_8_Pin|USR_IO_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : IPM_2_Pin IPM_6_Pin IPM_12_Pin IPM_9_Pin
                           ALARM_A_Pin USR_IO_18_Pin USR_IO_19_Pin */
  GPIO_InitStruct.Pin = IPM_2_Pin|IPM_6_Pin|IPM_12_Pin|IPM_9_Pin
                          |ALARM_A_Pin|USR_IO_18_Pin|USR_IO_19_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : IPMB_B_EN_Pin IPMB_A_EN_Pin */
  GPIO_InitStruct.Pin = IPMB_B_EN_Pin|IPMB_A_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : IPMB_B_RDY_Pin */
  GPIO_InitStruct.Pin = IPMB_B_RDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(IPMB_B_RDY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : IPMB_A_RDY_Pin HW_5_Pin HW_7_Pin */
  GPIO_InitStruct.Pin = IPMB_A_RDY_Pin|HW_5_Pin|HW_7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

  /*Configure GPIO pins : HW_3_Pin HW_1_Pin HW_6_Pin HW_4_Pin */
  GPIO_InitStruct.Pin = HW_3_Pin|HW_1_Pin|HW_6_Pin|HW_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pins : IPM_15_Pin MASTER_TDI_Pin MASTER_TMS_Pin USR_IO_11_Pin
                           USR_IO_31_Pin USR_IO_15_Pin USR_IO_13_Pin USR_IO_16_Pin */
  GPIO_InitStruct.Pin = IPM_15_Pin|MASTER_TDI_Pin|MASTER_TMS_Pin|USR_IO_11_Pin
                          |USR_IO_31_Pin|USR_IO_15_Pin|USR_IO_13_Pin|USR_IO_16_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pins : USR_IO_3_Pin MASTER_TCK_Pin PWR_GOOD_A_Pin USR_IO_32_Pin
                           USR_IO_30_Pin */
  GPIO_InitStruct.Pin = USR_IO_3_Pin|MASTER_TCK_Pin|PWR_GOOD_A_Pin|USR_IO_32_Pin
                          |USR_IO_30_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : ETH_TX_ER_Pin */
  GPIO_InitStruct.Pin = ETH_TX_ER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ETH_TX_ER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : HW_2_Pin HW_0_Pin */
  GPIO_InitStruct.Pin = HW_2_Pin|HW_0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : ALARM_B_Pin MASTER_TDO_Pin MASTER_TRST_Pin */
  GPIO_InitStruct.Pin = ALARM_B_Pin|MASTER_TDO_Pin|MASTER_TRST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : EN_12V_Pin FP_LED_2_Pin FP_LED_0_Pin FP_LED_1_Pin
                           FP_LED_BLUE_Pin */
  GPIO_InitStruct.Pin = EN_12V_Pin|FP_LED_2_Pin|FP_LED_0_Pin|FP_LED_1_Pin
                          |FP_LED_BLUE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PYLD_RESET_Pin */
  GPIO_InitStruct.Pin = PYLD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PYLD_RESET_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

}

/* USER CODE BEGIN 4 */


void KeyboardTask(void *argument)
{
  char c;
  fru_transition_t fru_trigg_val;

  // Start receiving bytes from keyboard
  uart4_input_stram = xStreamBufferCreate(10, 1);
  HAL_UART_Receive_IT(&huart4, &uart4_input_char, 1);

  for(;;)
  {
	xStreamBufferReceive( uart4_input_stram, &c, 1, portMAX_DELAY);

	ipmc_ios_printf("Pressed Key: %c\r\n", c);
	switch (c){
		case 'a':
			ipmc_ios_printf("ADDR: %x\r\n", ipmb_0_addr);
			break;
		case 'c':
			fru_trigg_val = CLOSE_HANDLE;
			xQueueSendToBack(queue_fru_transitions, &fru_trigg_val, 0UL);
			break;
		case 'o':
			fru_trigg_val = OPEN_HANDLE;
			xQueueSendToBack(queue_fru_transitions, &fru_trigg_val, 0UL);
			break;
		default:
			break;
	}

  }
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  char c;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  if( UartHandle ==  &huart4 )
  {
	xStreamBufferSendFromISR( uart4_input_stram, &uart4_input_char, 1, &xHigherPriorityTaskWoken);
	HAL_UART_Receive_IT(&huart4, &uart4_input_char, 1);
  }



}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if( hspi == &hspi4 )
		amc_gpios_spi_interrupt();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Informs the AMC_GPIOs driver that a falling edge happened in AMC_SPI_INT signal
	if( GPIO_Pin == GPIO_PIN_3)
		if( amc_gpios_pin_interrupt_task_handle != NULL )
			vTaskNotifyGiveFromISR( amc_gpios_pin_interrupt_task_handle, &xHigherPriorityTaskWoken );

}


/*
 * Callback to attend interrupts generated by the the AMC_IOs
 *
 * This callback runs in a FreeRTOS task context. This is NOT a CPU
 * interrupt!
 *
 */
void amc_gpios_pin_interrupt_callback( amc_int_status_t* interrupt_status )
{

	// Example of interrupt check
	if(interrupt_status[AMC2_IO_6] == AMC_INT_STATUS_FALLED)
	{
		asm("nop");
	}
	if(interrupt_status[AMC6_IO_2] == AMC_INT_STATUS_RISED)
	{
		asm("nop");
	}
}


/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  openipmc_hal_init();

  amc_gpios_init();


  /* Infinite loop */
  for(;;)
  {
    osDelay(500);
    LED_2_SET_STATE(SET);

    osDelay(500);
    LED_2_SET_STATE(RESET);

    //ipmc_ios_printf("Blink\n\r");

  }
  /* USER CODE END 5 */
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM7) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
