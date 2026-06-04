/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#include <stm32f4xx_hal_can.h>
#include <stm32f4xx_hal_rng.h>
#include <stm32f4xx_hal_rtc.h>
#include <stm32f4xx_hal_spi.h>
#include <stm32f4xx_hal_tim.h>
#include <stm32f4xx_hal_uart.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_task_scheduler.h"
#include "test_pwm.h"
#include "test_can.h"
#include "test_key.h"
#include "test_nvm.h"
#include "test_heat.h"
#include "stdio.h"   // For printf
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#ifdef __GNUC__
  /* With GCC, small printf (option ": " in linker options) calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define KEY1_Pin GPIO_PIN_3
#define KEY1_GPIO_Port GPIOE
#define KEY1_EXTI_IRQn EXTI3_IRQn
#define KEY0_Pin GPIO_PIN_4
#define KEY0_GPIO_Port GPIOE
#define KEY0_EXTI_IRQn EXTI4_IRQn
#define LIN1_TX_Pin GPIO_PIN_2
#define LIN1_TX_GPIO_Port GPIOA
#define LIN1_RX_Pin GPIO_PIN_3
#define LIN1_RX_GPIO_Port GPIOA
#define W25Q16_CS_Pin GPIO_PIN_0
#define W25Q16_CS_GPIO_Port GPIOB
#define LIN2_TX_Pin GPIO_PIN_10
#define LIN2_TX_GPIO_Port GPIOB
#define LIN2_RX_Pin GPIO_PIN_11
#define LIN2_RX_GPIO_Port GPIOB
#define LED_PWM_Pin GPIO_PIN_12
#define LED_PWM_GPIO_Port GPIOD
#define MOTOR_PWM_Pin GPIO_PIN_13
#define MOTOR_PWM_GPIO_Port GPIOD
#define MOTOR_DIR_Pin GPIO_PIN_14
#define MOTOR_DIR_GPIO_Port GPIOD
#define CAN1_STBY_Pin GPIO_PIN_8
#define CAN1_STBY_GPIO_Port GPIOA
#define SYS_JTCK_SWCLK_Pin GPIO_PIN_14
#define SYS_JTCK_SWCLK_GPIO_Port GPIOA
#define DS18B20_DQ_Pin GPIO_PIN_0
#define DS18B20_DQ_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

extern RNG_HandleTypeDef hrng;

extern RTC_HandleTypeDef hrtc;

extern SPI_HandleTypeDef hspi1;

extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
