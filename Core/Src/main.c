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
int nowAlarmType = AlarmTypeLow;
int oldAlarmType = AlarmTypeLow;
int alarmValue = 10;
int alarmValueManual = 30;
int newAlarmValueManual = 30;
uint8_t y = 0;
uint8_t x = 0;

extern uint16_t testbuffer[2];
extern uint16_t adc_buffer[1024];
extern uint16_t adc_buffer_cnt;

extern DMA_HandleTypeDef hdma_adc1;
#define SW1 HAL_GPIO_ReadPin(Switch1_GPIO_Port, Switch1_Pin)
#define SW2 HAL_GPIO_ReadPin(Switch2_GPIO_Port, Switch2_Pin)
#define UP_PRESS HAL_GPIO_ReadPin(BTN_UP_GPIO_Port, BTN_UP_Pin) == 0
#define DOWN_PRESS HAL_GPIO_ReadPin(BTN_DOWN_GPIO_Port, BTN_DOWN_Pin) == 0
#define OK_PRESS HAL_GPIO_ReadPin(BTN_OK_GPIO_Port, BTN_OK_Pin) == 0
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void showAlarmTypeLow() {
    oled_ShownChinese(0x00, 0x04, type_low_words[0]);
    oled_ShownChinese(0x10, 0x04, type_low_words[1]);
    oled_ShownEnglish(0x20, 0x04, type_low_words[2]); //:
    oled_ShownEnglish(0x29, 0x04, type_low_words[3]); //5
    oled_ShownChinese(0x30, 0x04, type_low_words[4]);
    oled_ShownEnglish(0x40, 0x04, type_low_words[5]);
    oled_ShownChinese(0x49, 0x04, type_low_words[6]);
    oled_ShownEnglish(0x59, 0x04, type_low_words[7]);
    oled_ShownEnglish(0x60, 0x04, type_low_words[8]);
    oled_ShownEnglish(0x69, 0x04, type_low_words[9]);
    oled_ShownChinese(0x70, 0x04, type_low_words[10]);
}
void showAlarmTypeHigh() {
    oled_ShownChinese(0x00, 0x04, type_high_words[0]);
    oled_ShownChinese(0x10, 0x04, type_high_words[1]);
    oled_ShownEnglish(0x20, 0x04, type_high_words[2]); //:
    oled_ShownEnglish(0x29, 0x04, type_high_words[3]); //5
    oled_ShownChinese(0x30, 0x04, type_high_words[4]);
    oled_ShownEnglish(0x40, 0x04, type_high_words[5]);
    oled_ShownChinese(0x49, 0x04, type_high_words[6]);
    oled_ShownEnglish(0x59, 0x04, type_high_words[7]);
    oled_ShownEnglish(0x60, 0x04, type_high_words[8]);
    oled_ShownEnglish(0x69, 0x04, type_high_words[9]);
    oled_ShownChinese(0x70, 0x04, type_high_words[10]);
}
void showAlarmTypeManual() {
    oled_ShownChinese(0x00, 0x04, type_manual_words[0]);
    oled_ShownChinese(0x10, 0x04, type_manual_words[1]);
    oled_ShownEnglish(0x20, 0x04, type_manual_words[2]); //:
    oled_ShownChinese(0x29, 0x04, type_manual_words[3]);
    oled_ShownChinese(0x39, 0x04, type_manual_words[4]);
    oled_ShownChinese(0x49, 0x04, type_manual_words[5]);
    oled_ShownChinese(0x59, 0x04, type_manual_words[6]);
    oled_ShownChinese(0x69, 0x04, type_manual_words[7]);
    oled_ShownEnglish(0x79, 0x04, type_manual_words[8]);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
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
    float32_t rms_303 = 0.0f;
    HAL_Delay(200);
    OLED_init();
    OLED_full();
    HAL_Delay(200);
    //    int fps = 0;
    char str[20];

    RetargetInit(&huart1);
    OLED_full();
    oled_ShownChinese(0x00, 0x00, Chinese_word[6]);
    oled_ShownChinese(0x10, 0x00, Chinese_word[7]);
    oled_ShownChinese(0x20, 0x00, Chinese_word[0]);
    oled_ShownChinese(0x30, 0x00, Chinese_word[1]);
    oled_ShownChinese(0x00, 0x02, Chinese_word[2]);
    oled_ShownChinese(0x10, 0x02, Chinese_word[3]);

    showAlarmTypeLow();

    //    oled_ShownChinese(0x10, 0x04, type_low_words[1]);
    sprintf(str, ":%dV    ", alarmValue);
    oled_write_string(0x40, 0x00, str, 8);

    while (1) {
        //get alarm type
        if (SW1 == 0) {
            nowAlarmType = AlarmTypeLow;
            alarmValue = 10;
            showAlarmTypeLow();
        } else if (SW2 == 0) {
            nowAlarmType = AlarmTypeManual;
            alarmValue = alarmValueManual;
            showAlarmTypeManual();
        } else {
            nowAlarmType = AlarmTypeHigh;
            alarmValue = 30;
            showAlarmTypeHigh();
        }

        //refresh the oled with new alarm type
        if (nowAlarmType != oldAlarmType) {
            oldAlarmType = nowAlarmType;
            // if (nowAlarmType == AlarmTypeLow) {
            // } else if (nowAlarmType == AlarmTypeHigh) {
            // } else {
            // }
            sprintf(str, ":%dV      ", alarmValue);
            oled_write_string(0x40, 0x00, str, 8);
        }

        //change the alarm value
        if (UP_PRESS) {
            HAL_Delay(20);
            if (UP_PRESS && nowAlarmType == AlarmTypeManual) {
                if (newAlarmValueManual < 250) {
                    newAlarmValueManual += 5;
                    sprintf(str, "%dV ", newAlarmValueManual);
                    oled_write_string_Over(0x48, 0x00, str, 4);
                }
            }
        }
        if (DOWN_PRESS) {
            HAL_Delay(20);
            if (DOWN_PRESS && nowAlarmType == AlarmTypeManual) {
                if (newAlarmValueManual > 20) {
                    newAlarmValueManual -= 5;
                    sprintf(str, "%dV ", newAlarmValueManual);
                    oled_write_string_Over(0x48, 0x00, str, 4);
                }
            }
        }
        if (OK_PRESS) {
            HAL_Delay(20);
            if (OK_PRESS && nowAlarmType == AlarmTypeManual) {
                alarmValueManual = newAlarmValueManual;
                sprintf(str, ":%dV   ", alarmValueManual);
                oled_write_string(0x40, 0x00, str, strlen(str));
            }
        }

        //adc work
        if (adc_buffer_cnt == 1024) {
            for (int i = 0; i < 1024; i++) {
                // rms_buffer[i] = adc_buffer[i] / 4096.0 * 3.3 - (3.3 / (750 + 240) * 240 * 2); //1:(3+1),2 Maginify
                rms_buffer[i] = adc_buffer[i] / 4096.0 * 3.3 - 1.754; //1:(3+1),2 Maginify
                // printf("%f\n", rms_buffer[i]);
            }
            float square_buffer[1024]; // 存储平方值
            float mean_square;
            arm_mult_f32(rms_buffer, rms_buffer, square_buffer, 1024);

            // 3. 计算平方的平均值
            arm_mean_f32(square_buffer, 1024, &mean_square);

            // 4. 计算有效值（RMS）
            arm_sqrt_f32(mean_square, &rms_303);
            adc_buffer_cnt=0;
        }

        //calculate the alarm value
        double rms_220 = (rms_303) / 0.0309 * 11.0309 / 750.0 * (750 + 240 + 30.9) / 2.0; //0.0309:11,1:3,2Maginify
        printf("%f\r\n", rms_220);
        // rms_220 = rms_220 * 1.085;
        //check is need alarm
        if ((int) rms_220 > alarmValue) {
            //                HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, 1);
            //                ssd1306_SetCursor(2, 50);
            htim3.Instance->CCR2 = 250;
            sprintf(str, "  ALARMING!!!!  ");
            oled_write_string(0x00, 0x06, str, strlen(str));
        } else {
            //                HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, 0);
            htim3.Instance->CCR2 = 0;
            //                ssd1306_SetCursor(2, 50);
            sprintf(str, "  WORKING....   ");
            oled_write_string(0x00, 0x06, str, strlen(str));
        }
        //            ssd1306_SetCursor(30, 24);
        sprintf(str, ":%.1fV        ", rms_220);
        oled_write_string(0x20, 0x02, str, 12);

        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {
    }
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while (LL_RCC_HSE_IsReady() != 1) {
    }
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1) {
    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    }
    LL_SetSystemCoreClock(72000000);

    /* Update the time base */
    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK) {
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
void Error_Handler(void) {
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
