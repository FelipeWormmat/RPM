/*
 * Application.h
 *
 *  Created on: Jul 27, 2020
 *      Author: Felipe Wormmat
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_
/* Exported includes ----------------------------------------------------------*/
#include "main.h"
#include "../Include/sys/_stdint.h"
/* Exported defines ----------------------------------------------------------*/
#define false 0
#define true 1

#define NUM_LIN_LCD 2
#define NUM_COL_LCD 8

#define kTimerAppDisplayTask 350
#define kTimerAppInputTask 50
#define kTimerAppTask 350
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void ApplicationInit(void);
void AppTimeTask(void);
void Atualiza_LCD(void);
void ApplicationTask(void);
void AppCapture(uint16_t capture);
void AppSaveConfig(void);
#endif /* APPLICATION_H_ */
