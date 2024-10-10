/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "arm_math.h"
#include "retarget.h"
#include "ssd1306.h"
#include <stdbool.h>
#include "ssd1306_fonts.h"
#include "ssd1306_tests.h"
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

/* USER CODE BEGIN PV */
int thresholdType = ThresholdTypeLow;
int thresholdTypeOld = ThresholdTypeLow;
int threshod = 30;
int thresholdOld = 30;
uint8_t y = 0;

extern uint16_t testbuffer[2];
extern uint16_t adc_buffer[1024];
extern uint16_t adc_buffer_cnt;

extern DMA_HandleTypeDef hdma_adc1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint32_t Read_ADC_Channel(uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    uint32_t adc_value = 0;

    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
        Error_Handler();
    }

    HAL_ADC_Start(&hadc2);

    if (HAL_ADC_PollForConversion(&hadc2, HAL_MAX_DELAY) == HAL_OK) {
        adc_value = HAL_ADC_GetValue(&hadc2);
    }

    // 关闭ADC
    HAL_ADC_Stop(&hadc2);

    return adc_value;
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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_ADC2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    HAL_TIM_PWM_Start(&htim4,TIM_CHANNEL_4);
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADCEx_Calibration_Start(&hadc2);
    __HAL_DMA_ENABLE_IT(&hdma_adc1, DMA_IT_TC);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *) &testbuffer, 1);

    ssd1306_Init();
//    ssd1306_Fill(Black);

    float32_t rms_buffer[1024] = {0};
    volatile float BaseVolatge;
    float32_t Rms = 0.0f;
    ssd1306_TestCircle();
    while (1) {
        ssd1306_TestFPS();
//        ssd1306_SetCursor(2, 1);
//        char str[20];
//        sprintf(str, "ThresholdHigh:%d", threshod);
////        ssd1306_WriteString(str, Font_16x24, White);
////        ssd1306_SetCursor(2, 25);
////        sprintf(str, "ThresholdLow:%d", thresholdType*25+5);
////        ssd1306_WriteString(str, Font_16x24, White);
//        thresholdType = HAL_GPIO_ReadPin(switch_GPIO_Port, switch_Pin);
//        if (thresholdType != thresholdTypeOld) {
//            thresholdTypeOld = thresholdType;
//            if (thresholdType == ThresholdTypeHigh) {
//                threshod = 30;
//            } else if (thresholdType == ThresholdTypeLow) {
//                threshod = 5;
//            }
//        }
//        if (HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin) == 0) {
//            HAL_Delay(10);
//            if (HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin) == 0) {
//                if (thresholdType == ThresholdTypeHigh && threshod < 250) {
//                    threshod += 10;
//
//                } else if (thresholdType == ThresholdTypeLow && threshod < 30) {
//                    threshod += 1;
//                }
//            }
//        }
//        if (HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port,BTN_DOWN_Pin)==0){
//            HAL_Delay(10);
//            if (HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port,BTN_DOWN_Pin)==0){
//                if (thresholdType == ThresholdTypeHigh && threshod > 30) {
//                    threshod -= 10;
//                } else if (thresholdType == ThresholdTypeLow && threshod > 5) {
//                    threshod -= 1;
//                }
//            }
//        }
//        if (HAL_GPIO_ReadPin(BTN_OK_GPIO_Port,BTN_OK_Pin)==0){
//            HAL_Delay(10);
//            if (HAL_GPIO_ReadPin(BTN_OK_GPIO_Port,BTN_OK_Pin)==0){
//                 thresholdOld = threshod;
//            }
//        }
//        if (adc_buffer_cnt == 1024) {
//            uint32_t adcValue1 = Read_ADC_Channel(ADC_CHANNEL_2);
//            BaseVolatge = (adcValue1) / 4096.0 * 3.3;
//            for (int i = 0; i < 1024; i++) {
//                rms_buffer[i] = adc_buffer[i] / 4096.0 * 3.3 - (BaseVolatge / 2.0 * 2.5);
//            }
//            int zeroCrossings[10];
//            int zeroCrossingCount = 0;
//            // 查找正向零交叉点
//            for (int i = 0; i < 1023; i++) {
//                if (rms_buffer[i] <= 0 && rms_buffer[i + 1] > 0) {
//                    zeroCrossings[zeroCrossingCount++] = i;
//                    if (zeroCrossingCount >= 10) {
//                        break;
//                    }
//                }
//            }
//            int startIdx, endIdx, length = 0;
//            if (zeroCrossingCount >= 3) {
//                startIdx = zeroCrossings[0]; // 第一个零交叉点的索引
//                endIdx = zeroCrossings[2]; // 第三个零交叉点的索引，对�??2个完整周�??
//                length = endIdx - startIdx; // 计算2个周期的长度
//
//                arm_rms_f32(&rms_buffer[startIdx], length, &Rms);
//            }
//            adc_buffer_cnt = 0;
//        }
//            if (thresholdType == ThresholdTypeHigh) {
//                HAL_GPIO_WritePin(RELAY_CTRL_GPIO_Port,RELAY_CTRL_Pin,0);
//                float RMSN = (Rms) * sqrt(2) / 0.47 * 220 / 1.25 / 9.45 * 10.0;
//                if (RMSN > thresholdOld){
//                    HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,0);
//                    ssd1306_SetCursor(2, 20);
//                    sprintf(str, "ALIARMING!!!!");
////                    ssd1306_WriteString(str, Font_16x24, White);
//                } else{
//                    HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,0);
//                    ssd1306_SetCursor(2, 20);
//                    sprintf(str, "             ");
////                    ssd1306_WriteString(str, Font_16x24, White);
//                }
//                HAL_GPIO_WritePin(RELAY_CTRL_GPIO_Port,RELAY_CTRL_Pin,1);
//        } else if (thresholdType == ThresholdTypeLow) {
//            HAL_GPIO_WritePin(RELAY_CTRL_GPIO_Port,RELAY_CTRL_Pin,1);
//            float RMSN = (Rms) * sqrt(2) / 0.47 * 22.0 / 1.25 / 9.45 * 10.0;
//            if (RMSN > thresholdOld){
//                HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,1);
//                ssd1306_SetCursor(2, 49);
//                sprintf(str, "ALIARMING!!!!");
////                ssd1306_WriteString(str, Font_16x24, White);
//            } else{
//                HAL_GPIO_WritePin(BEEP_GPIO_Port,BEEP_Pin,0);
//                ssd1306_SetCursor(2, 49);
//                sprintf(str, "             ");
////                ssd1306_WriteString(str, Font_16x24, White);
//            }
//        }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//        ssd1306_UpdateScreen();
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(72000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSRC_PCLK2_DIV_6);
}

/* USER CODE BEGIN 4 */

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
    while (1) {
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
