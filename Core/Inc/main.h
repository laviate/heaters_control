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

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

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
#define MDI_OCALERT_Pin GPIO_PIN_14
#define MDI_OCALERT_GPIO_Port GPIOC
#define DB7_Pin GPIO_PIN_0
#define DB7_GPIO_Port GPIOA
#define DB6_Pin GPIO_PIN_1
#define DB6_GPIO_Port GPIOA
#define DB5_Pin GPIO_PIN_2
#define DB5_GPIO_Port GPIOA
#define DB4_Pin GPIO_PIN_3
#define DB4_GPIO_Port GPIOA
#define DB3_Pin GPIO_PIN_4
#define DB3_GPIO_Port GPIOA
#define DB2_Pin GPIO_PIN_5
#define DB2_GPIO_Port GPIOA
#define DB1_Pin GPIO_PIN_6
#define DB1_GPIO_Port GPIOA
#define DB0_Pin GPIO_PIN_7
#define DB0_GPIO_Port GPIOA
#define MDO_RW_Pin GPIO_PIN_0
#define MDO_RW_GPIO_Port GPIOB
#define MDO_RS_Pin GPIO_PIN_1
#define MDO_RS_GPIO_Port GPIOB
#define TEMP_CONTROL_Pin GPIO_PIN_8
#define TEMP_CONTROL_GPIO_Port GPIOA
#define BUTTON1_Pin GPIO_PIN_9
#define BUTTON1_GPIO_Port GPIOA
#define BUTTON1_EXTI_IRQn EXTI4_15_IRQn
#define BUTTON2_Pin GPIO_PIN_10
#define BUTTON2_GPIO_Port GPIOA
#define BUTTON2_EXTI_IRQn EXTI4_15_IRQn
#define ALERT4_Pin GPIO_PIN_11
#define ALERT4_GPIO_Port GPIOA
#define ALERT4_EXTI_IRQn EXTI4_15_IRQn
#define ALERT3_Pin GPIO_PIN_12
#define ALERT3_GPIO_Port GPIOA
#define ALERT3_EXTI_IRQn EXTI4_15_IRQn
#define ALERT2_Pin GPIO_PIN_15
#define ALERT2_GPIO_Port GPIOA
#define ALERT2_EXTI_IRQn EXTI4_15_IRQn
#define ALERT1_Pin GPIO_PIN_3
#define ALERT1_GPIO_Port GPIOB
#define ALERT1_EXTI_IRQn EXTI2_3_IRQn
#define SCALERT_Pin GPIO_PIN_4
#define SCALERT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
