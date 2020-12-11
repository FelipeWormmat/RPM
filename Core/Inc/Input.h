/*
 * Input.h
 *
 *  Created on: Aug 1, 2020
 *      Author: Felipe Wormmat
 */

#ifndef SRC_INPUT_H_
#define SRC_INPUT_H_
/* Exported includes ----------------------------------------------------------*/
#include "main.h"
/* Exported defines ----------------------------------------------------------*/
#define kInputCount 4U
/* Exported types ------------------------------------------------------------*/
typedef enum
{
	eInputReset,
	eInputHour,
	eInputCycle,
	eInputA1,
	eInputA3,
	eInputExit1,
	eInputExit2
} InputNameType;

typedef enum
{
	ANALOG = 0,
	DIGITAL = 1,
} InputFunctionType;

typedef enum
{
	InOFF = 0,
	InON = 1
} InputStateType;

typedef struct
{
	GPIO_PinState InState;
	GPIO_TypeDef *GPIO;
	uint16_t GPIO_Pin;
	uint8_t InDebounceUp;
	uint8_t InDebounceDown;
	uint8_t InDebounceUpCounter;
	uint8_t InDebounceDownCounter;
	GPIO_PinState InStateAtt;
	GPIO_PinState InStateOld;
	InputFunctionType InFunctionType;
	uint16_t InAdcValue;
} InputStruct;

InputStruct Input;
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
InputStruct InputConf[kInputCount];

uint32_t AdcDmaBuffer[kInputCount];
uint32_t AdcDmaAcc[kInputCount];
uint32_t AdcDmaAverageBuffer[kInputCount];
/* Exported functions prototypes ---------------------------------------------*/
void InputsInit(void);
uint8_t InputsGetState(InputNameType InName);
void InputsPeriodic(void);
uint16_t InputsGetValue(InputNameType InName);
void InputsAdcPeriodic(void);

#endif /* SRC_INPUT_H_ */
