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
#include "stm32f0xx_hal.h"

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
void DelayUs(uint16_t Us);
void msTmr(void);
void intTmr14(void);
void IWDG_Refresh(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MCU_IN_A1_Pin GPIO_PIN_0
#define MCU_IN_A1_GPIO_Port GPIOA
#define MCU_EXIT_VLOW_Pin GPIO_PIN_1
#define MCU_EXIT_VLOW_GPIO_Port GPIOA
#define MCU_EXIT_VLOW_EXTI_IRQn EXTI0_1_IRQn
#define MCU_IN_E1_Pin GPIO_PIN_2
#define MCU_IN_E1_GPIO_Port GPIOA
#define MCU_IN_E2_Pin GPIO_PIN_3
#define MCU_IN_E2_GPIO_Port GPIOA
#define MCU_DIS_RS_Pin GPIO_PIN_5
#define MCU_DIS_RS_GPIO_Port GPIOA
#define MCU_DIS_EN_Pin GPIO_PIN_6
#define MCU_DIS_EN_GPIO_Port GPIOA
#define MCU_DIS_D5_Pin GPIO_PIN_7
#define MCU_DIS_D5_GPIO_Port GPIOA
#define MCU_DIS_D4_Pin GPIO_PIN_1
#define MCU_DIS_D4_GPIO_Port GPIOB
#define MCU_DIS_D6_Pin GPIO_PIN_9
#define MCU_DIS_D6_GPIO_Port GPIOA
#define MUC_DIS_D7_Pin GPIO_PIN_10
#define MUC_DIS_D7_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
