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
int range = 30;
int threshold = 20;
int thresholdOld = 20;
uint8_t y = 0;
uint8_t x = 0;

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

uint32_t Read_ADC_Channel(uint32_t channel)
{
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

    // �ر�ADC
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
  MX_ADC2_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADCEx_Calibration_Start(&hadc2);
    __HAL_DMA_ENABLE_IT(&hdma_adc1, DMA_IT_TC);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *) &testbuffer, 1);
    float32_t rms_buffer[1024] = {0};
    volatile float BaseVolatge;
    float32_t Rms = 0.0f;

    OLED_init();
    OLED_full();
    HAL_Delay(200);
    int fps = 0;
    char str[20];
    HAL_GPIO_WritePin(RELAY_CTRL_GPIO_Port, RELAY_CTRL_Pin, 1);
    RetargetInit(&huart1);
    OLED_init();
    OLED_full();
    HAL_Delay(300);
    oled_ShownChinese(0x00, 0x00, Chinese_word[6]);
    oled_ShownChinese(0x10, 0x00, Chinese_word[7]);
    oled_ShownChinese(0x20, 0x00, Chinese_word[0]);
    oled_ShownChinese(0x30, 0x00, Chinese_word[1]);
    oled_ShownChinese(0x00, 0x02, Chinese_word[2]);
    oled_ShownChinese(0x10, 0x02, Chinese_word[3]);
    oled_ShownChinese(0x00, 0x04, Chinese_word[4]);
    oled_ShownChinese(0x10, 0x04, Chinese_word[5]);

    while (1) {
        sprintf(str, ":%dV-%dV  ", thresholdType *(-25) + 30, range);
        oled_write_string(0x20, 0x04, str, strlen(str) );//��ӡ����

        //�ж��Ƿ���Ҫ�е�λ
        thresholdType = HAL_GPIO_ReadPin(switch_GPIO_Port, switch_Pin);
        if (thresholdType != thresholdTypeOld) {
            thresholdTypeOld = thresholdType;
            if (thresholdType == ThresholdTypeHigh) {
                HAL_GPIO_WritePin(RELAY_CTRL_GPIO_Port, RELAY_CTRL_Pin, 0);
                range = 250;
                threshold = 100;
            } else if (thresholdType == ThresholdTypeLow) {
                HAL_GPIO_WritePin(RELAY_CTRL_GPIO_Port, RELAY_CTRL_Pin, 1);
                range = 30;
                threshold = 20;
            }
        }
        sprintf(str, ":%dV ", threshold);//���µ�λ���ݴ�ӡ
        oled_write_string(0x40,0x00,str, strlen(str));

        //��ֵ�������?
        if (HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin) == 0) {
            HAL_Delay(20);
            if (HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin) == 0) {
                if (thresholdType == ThresholdTypeHigh && threshold < 250) {
                    threshold += 10;

                } else if (thresholdType == ThresholdTypeLow && threshold < 30) {
                    threshold += 1;
                }
                sprintf(str, ":%dV  ", threshold);
                oled_write_string_Over(0x40,0x00,str, strlen(str));
            }
        }
        if (HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin) == 0) {
            HAL_Delay(20);
            if (HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin) == 0) {
                if (thresholdType == ThresholdTypeHigh && threshold > 30) {
                    threshold -= 10;
                } else if (thresholdType == ThresholdTypeLow && threshold > 5) {
                    threshold -= 1;
                }
                sprintf(str, ":%dV  ", threshold);
                oled_write_string_Over(0x40,0x00,str, strlen(str));
            }
        }
        if (HAL_GPIO_ReadPin(BTN_OK_GPIO_Port, BTN_OK_Pin) == 0) {
            HAL_Delay(20);
            if (HAL_GPIO_ReadPin(BTN_OK_GPIO_Port, BTN_OK_Pin) == 0) {
                thresholdOld = threshold;
                sprintf(str, ":%dV   ", thresholdOld);
                oled_write_string(0x40,0x00,str, strlen(str));
            }
        }

        //ADC��������
        if (adc_buffer_cnt == 1024) {
//            uint32_t adcValue1 = Read_ADC_Channel(ADC_CHANNEL_2);
//            BaseVolatge = (adcValue1) / 4096.0 * 3.3;
            for (int i = 0; i < 1024; i++) {
                rms_buffer[i] = adc_buffer[i] / 4096.0 * 3.3 - (3.3 / 4.0 * 2);
//                printf("%f\n", rms_buffer[i]);
            }
            int zeroCrossings[10];
            int zeroCrossingCount = 0;
            // ���������㽻���?
            for (int i = 0; i < 1023; i++) {
                if (rms_buffer[i] <= 0 && rms_buffer[i + 1] > 0) {
                    zeroCrossings[zeroCrossingCount++] = i;
                    if (zeroCrossingCount >= 10) {
                        break;
                    }
                }
            }
            int startIdx, endIdx, length = 0;
            if (zeroCrossingCount >= 3) {
                startIdx = zeroCrossings[0]; // ��һ���㽻��������
                endIdx = zeroCrossings[2]; // �������㽻��������
                length = endIdx - startIdx; // ����2�����ڵĳ���

                arm_rms_f32(&rms_buffer[startIdx], length, &Rms);
            } else{
                Rms = 0;
            }
            adc_buffer_cnt = 0;
        }

        //���뵲λ��������
        if (thresholdType == ThresholdTypeHigh) {
            float RMSN = (Rms) / 0.47 * 220.0 / 0.75 / 2 * 0.99;
            if (RMSN > thresholdOld) {//�����ж�
//                HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, 1);
//                ssd1306_SetCursor(2, 50);
                htim3.Instance->CCR2 = 250;
                sprintf(str, "ALARMING!!!!");
                oled_write_string(0x10,0x06,str, strlen(str));
            } else {
//                HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, 0);
                htim3.Instance->CCR2 = 0;
//                ssd1306_SetCursor(2, 50);
                sprintf(str, "WORKING....  ");
                oled_write_string(0x10,0x06,str, strlen(str));
            }
//            ssd1306_SetCursor(30, 24);
            sprintf(str, ":%.1fV", RMSN, RMSN * sqrt(2));
            oled_write_string(0x20,0x02,str, strlen(str));
        } else if (thresholdType == ThresholdTypeLow) {
            float RMSN = (Rms) / 0.47 * 22.0 / 0.75 / 2 * 1.04;
            if (RMSN > thresholdOld) //�����ж�
            {
//                HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, 1);
//                ssd1306_SetCursor(2, 50);
                htim3.Instance->CCR2 = 250;
                sprintf(str, "ALARMING!!!!");
                oled_write_string(0x10,0x06,str, strlen(str));
            } else {
                htim3.Instance->CCR2 = 0;
                sprintf(str, "WORKING....  ");
                oled_write_string(0x10,0x06,str, strlen(str));
            }
            sprintf(str, ":%.1fV", RMSN, RMSN * sqrt(2));
            oled_write_string(0x20,0x02,str, strlen(str));

        }

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
