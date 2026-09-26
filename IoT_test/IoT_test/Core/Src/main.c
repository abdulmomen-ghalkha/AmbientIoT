/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
//#define SHT40_ADDR (0x44 << 1) // SHT40 7-bit I2C address shifted left
//uint8_t sht40_cmd = 0xFD; // High precision read command
//uint8_t sht40_rx[6]; // Buffer for Temp (2B) + CRC (1B) + Hum (2B) + CRC (1B)

#define SHT31_ADDR (0x44 << 1) // SHT31 7-bit I2C address shifted left
uint8_t sht31_cmd[2] = {0x24, 0x00}; // SHT31 High precision read, clock stretching disabled
uint8_t sht31_rx[6];           // Buffer for Temp (2B) + CRC (1B) + Hum (2B) + CRC (1B)

uint16_t adc_raw_val = 0;
float temperature = 0.0;
float humidity = 0.0;
char tx_buffer[100];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include <stdio.h>
#include <string.h>

#define MOISTURE_VCC_Pin GPIO_PIN_1
#define MOISTURE_VCC_Port GPIOB
#define BLE_EN_Pin       GPIO_PIN_4

// Redirect printf to USART2 (Debug/PC/CoolTerm)
int __io_putchar(int ch) {
//    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}


//void Read_Sensors(void) {
//	// Clear buffer
//	for(int i=0; i<6; i++) sht40_rx[i] = 0;
//
//	// 1. Read SHT40
//	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, SHT40_ADDR, &sht40_cmd, 1, 100);
//	if (status != HAL_OK) {
//		printf("[DEBUG] I2C Transmit Error Code: %d\r\n", status);
//	}
//
//	HAL_Delay(15);
//
//	status = HAL_I2C_Master_Receive(&hi2c1, SHT40_ADDR, sht40_rx, 6, 100);
//	if (status != HAL_OK) {
//		printf("[DEBUG] I2C Receive Error Code: %d\r\n", status);
//	} else {
//		// Print raw hex data to the console
//		printf("[DEBUG] Raw Bytes: %02X %02X %02X %02X %02X %02X\r\n",
//			   sht40_rx[0], sht40_rx[1], sht40_rx[2], sht40_rx[3], sht40_rx[4], sht40_rx[5]);
//	}
//
//    // Convert Raw SHT40 Data
//    uint16_t t_ticks = (sht40_rx[0] << 8) | sht40_rx[1];
//    uint16_t rh_ticks = (sht40_rx[3] << 8) | sht40_rx[4];
//    temperature = -45.0f + 175.0f * ((float)t_ticks / 65535.0f);
//    humidity = -6.0f + 125.0f * ((float)rh_ticks / 65535.0f);
//
//    // 2. Power up & Read Analog Soil Moisture
//    // Power up the sensor using the correct Port (GPIOB)
//    HAL_GPIO_WritePin(MOISTURE_VCC_Port, MOISTURE_VCC_Pin, GPIO_PIN_SET);
//    HAL_Delay(10); // Allow sensor voltage to stabilize
//
//    HAL_ADC_Start(&hadc1);
//    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
//        adc_raw_val = HAL_ADC_GetValue(&hadc1);
//    }
//    HAL_ADC_Stop(&hadc1);
//
//    HAL_GPIO_WritePin(MOISTURE_VCC_Port, MOISTURE_VCC_Pin, GPIO_PIN_RESET); // Power down sensor to prevent electrolysis
//}

void Read_Sensors(void) {
    // Clear buffer
    for(int i=0; i<6; i++) sht31_rx[i] = 0;

    // 1. Read SHT31 (Note the size changed to 2 bytes)
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, SHT31_ADDR, sht31_cmd, 2, 100);
    if (status != HAL_OK) {
        printf("[DEBUG] I2C Transmit Error Code: %d\r\n", status);
    }

    HAL_Delay(15); // SHT31 high repeatability takes up to ~15ms

    status = HAL_I2C_Master_Receive(&hi2c1, SHT31_ADDR, sht31_rx, 6, 100);
    if (status != HAL_OK) {
        printf("[DEBUG] I2C Receive Error Code: %d\r\n", status);
    } else {
        // Print raw hex data to the console
        printf("[DEBUG] Raw Bytes: %02X %02X %02X %02X %02X %02X\r\n",
               sht31_rx[0], sht31_rx[1], sht31_rx[2], sht31_rx[3], sht31_rx[4], sht31_rx[5]);
    }

    // Convert Raw SHT31 Data (The formula is identical to the SHT40)
    uint16_t t_ticks = (sht31_rx[0] << 8) | sht31_rx[1];
    uint16_t rh_ticks = (sht31_rx[3] << 8) | sht31_rx[4];
    temperature = -45.0f + 175.0f * ((float)t_ticks / 65535.0f);
    humidity = 100.0f * ((float)rh_ticks / 65535.0f); // Note: SHT31 humidity math is slightly different from SHT40

    // 2. Power up & Read Analog Soil Moisture
    // ... (Keep the rest of your moisture sensor code exactly the same)
    HAL_GPIO_WritePin(MOISTURE_VCC_Port, MOISTURE_VCC_Pin, GPIO_PIN_SET);
    HAL_Delay(10);

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        adc_raw_val = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);

    HAL_GPIO_WritePin(MOISTURE_VCC_Port, MOISTURE_VCC_Pin, GPIO_PIN_RESET);
}


void Transmit_Data(void) {
    // 1. Prepare payload
    char tx_buffer[32];
    int len = sprintf(tx_buffer, "%.1f,%.1f,%d", temperature, humidity, adc_raw_val);
    uint8_t stx = 0x02; // Start of Text
    uint8_t etx = 0x03; // End of Text

    // WAKE PING: Send dummy byte to wake Xiao
    uint8_t dummy = 0xFF;
    HAL_UART_Transmit(&huart1, &dummy, 1, 100);

    // Give the Xiao nRF52840 10 milliseconds to wake up
    HAL_Delay(10);

    // Now send the actual payload
    HAL_UART_Transmit(&huart1, &stx, 1, 100);
    HAL_UART_Transmit(&huart1, (uint8_t*)tx_buffer, len, 100);
    HAL_UART_Transmit(&huart1, &etx, 1, 100);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    printf("\r\n=== System Booted successfully ===\r\n");

    while (1)
    {
    	HAL_Delay(2000); // ADD THIS
        printf("[System] Starting sensor read step...\r\n");

        // 1. Read SHT40 and Moisture Sensor
        Read_Sensors();
        if (temperature == 0.0 && humidity == 0.0) {
            printf("[DEBUG] Sensor returned zero. Check I2C Wiring!\r\n");
        }

        printf("[System] Data read: T:%.1f, H:%.1f, S:%d\r\n", temperature, humidity, adc_raw_val);
        printf("[System] Transmitting via UART1 to BLE...\r\n");

//        HAL_Delay(2000);

        // 2. Transmit to Xiao BLE via USART1
        Transmit_Data();
//
        printf("[System] Transmission complete. Entering STOP2 Mode...\r\n");



		// === PRE-SLEEP ISOLATION ===

        // This guarantees MspInit() will restore the pins when we wake up.
        HAL_UART_DeInit(&huart1);
        HAL_I2C_DeInit(&hi2c1);
        HAL_ADC_DeInit(&hadc1);

		// Convert EVERY pin on Port A and Port B to Analog to kill all floating leaks.
		// (Our MX_Init functions will automatically restore the ones we need when we wake up!)
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = GPIO_PIN_All;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		// 1. Suspend the SysTick timer.
		// (If we don't do this, the standard 1ms timer might instantly wake us back up).
		HAL_SuspendTick();

		// Disabling debug mode consumption
		HAL_DBGMCU_DisableDBGStopMode();

		// 2. Enter STOP 2 Mode.
		// The CPU completely shuts down here. It consumes ~2µA.
		HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);

		// ==========================================================
		//  THE SYSTEM IS NOW ASLEEP.
		//  CODE EXECUTION PAUSES HERE.
		// ==========================================================

		// 3. The RTC Alarm rings. The CPU wakes up and resumes on this line.

		// 4. Wake up the SysTick timer.
		HAL_ResumeTick();

		// 5. Re-initialize the main clock.
		// (STOP2 mode disables the main MSI clock to save power. We must turn it back on).
		SystemClock_Config();

		// 6. Wake up the peripherals!
		MX_GPIO_Init();
		MX_USART1_UART_Init();
		MX_I2C1_Init();
		MX_ADC1_Init();

		printf("\r\n[System] Woke up from STOP2!\r\n");
////        HAL_Delay(2000);
////
////        printf("[System] Transmission complete. Sleeping for x s...\r\n");
//
//        HAL_Delay(20000);
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hi2c1.Init.Timing = 0x00100D14;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
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
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 30, RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PB1 LD3_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_1|LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    // The CPU wakes up and jumps here.
    // We don't need it to do anything; we just need it to wake up!
}

/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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
