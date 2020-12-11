/*
 * Input.c
 *
 *  Created on: Aug 1, 2020
 *      Author: Felipe Wormmats
 */
/* Private includes ----------------------------------------------------------*/
#include <Input.h>
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern ADC_HandleTypeDef hadc;
/* Private function prototypes -----------------------------------------------*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
void InputsConfigure(InputNameType InName, uint8_t InDebounceDown, uint8_t InDebounceUp, uint16_t GPIO_Pin, GPIO_TypeDef *GPIO, InputFunctionType InFunction);
/* Private user code ---------------------------------------------------------*/

/******************************************************************************/
/**
 * @brief contigure a input struct param for one each
 *
 * @param  InName				A enum index in vector
 * @param  InDebounceDown		A time for Bounce out
 * @param  InDebounceUp			A time for bounce in
 * @param  GPIO_Pin				A number of a port pin
 * @param  GPIO_TypeDef*		A pointer to a structure off a port
 */
void InputsConfigure(InputNameType InName, uint8_t InDebounceDown, uint8_t InDebounceUp, uint16_t GPIO_Pin, GPIO_TypeDef *GPIO, InputFunctionType InFunction)
{
	/* Check the parameters */
	if (InName < kInputCount)
	{
		InputConf[InName].GPIO = GPIO;
		InputConf[InName].GPIO_Pin = GPIO_Pin;
		InputConf[InName].InDebounceDown = InDebounceDown;
		InputConf[InName].InDebounceUp = InDebounceUp;
		InputConf[InName].InDebounceDownCounter = 0x00;
		InputConf[InName].InDebounceUpCounter = 0x00;
		InputConf[InName].InStateAtt = GPIO_PIN_RESET;
		InputConf[InName].InStateOld = GPIO_PIN_RESET;
		InputConf[InName].InFunctionType = InFunction;
		InputConf[InName].InAdcValue = 0x0000;
	}
}

/******************************************************************************/
/**
 * @brief Use a InputsConfigure for listing inputs avaliable
 * @param  None
 */
void InputsInit(void)
{
	/*todo Verificar os debounce*/
	InputsConfigure(eInputReset, 50U, 50U, MCU_IN_E1_Pin, MCU_IN_E1_GPIO_Port, DIGITAL);
	InputsConfigure(eInputHour, 5U, 5U, MCU_IN_E2_Pin, MCU_IN_E2_GPIO_Port, DIGITAL);
	//InputsConfigure(eInputCycle, 5U, 5U, MCU_IN_E3_Pin,  MCU_IN_E3_GPIO_Port, DIGITAL);
	/*Todo Adicionar a entrada Exit1 e Analogic 1
	 * Corrigir a constante kInputCount*/
	InputsConfigure(eInputA1, 0, 0, 0, 0, ANALOG);
}

/******************************************************************************/
/**
 * @brief Get a state off a digital input
 * @param  InName				A enum index in vector
 */
uint8_t InputsGetState(InputNameType InName)
{
	uint8_t InStateRet = 0;

	if (InName < kInputCount)
	{
		InStateRet = InputConf[InName].InStateAtt;
	}
	return (InStateRet);
}

/******************************************************************************/
/**
 * @brief Get a value off a analog input
 * @param  InName				A enum index in vector
 */
uint16_t InputsGetValue(InputNameType InName)
{
	uint16_t InValueRet = 0;

	if (InName < kInputCount)
	{
		InValueRet = InputConf[InName].InAdcValue;
	}
	return (InValueRet);
}

uint8_t InputGetValue(InputNameType InName)
{
	uint16_t InValueRet = 0;

	if (InName < kInputCount)
	{
		InValueRet = InputConf[InName].InAdcValue;
	}
	return (InValueRet);
}
/******************************************************************************/
/**
 * @brief Execute a filter and bounce off all inputs
 * @param  None

 */
void InputsAdcPeriodic(void)
{
	HAL_ADC_Start_DMA(&hadc, AdcDmaBuffer, kInputCount);
}

/******************************************************************************/
/**
 * @brief Execute a filter and bounce off all inputs
 * @param  None

 */
void InputsPeriodic(void)
{
	uint8_t i = 0;

	for (i = 0; i < kInputCount; i++)
	{
		if (InputConf[i].InFunctionType == DIGITAL)
		{
			InputConf[i].InState = HAL_GPIO_ReadPin(InputConf[i].GPIO, InputConf[i].GPIO_Pin);

			if (InputConf[i].InState == GPIO_PIN_RESET)
			{
				if (InputConf[i].InDebounceUpCounter < InputConf[i].InDebounceUp)
				{
					InputConf[i].InDebounceUpCounter++;
				}
				else
				{
					InputConf[i].InStateAtt = GPIO_PIN_SET;
					InputConf[i].InDebounceDownCounter = 0;
					InputConf[i].InStateOld = InputConf[i].InState;
				}
			}

			if (InputConf[i].InState == GPIO_PIN_SET)
			{
				if (InputConf[i].InDebounceDownCounter < InputConf[i].InDebounceDown)
				{
					InputConf[i].InDebounceDownCounter++;
				}
				else
				{
					InputConf[i].InStateAtt = GPIO_PIN_RESET;
					InputConf[i].InDebounceUpCounter = 0;
					InputConf[i].InStateOld = InputConf[i].InState;
				}
			}
		}
		else
		{
			InputConf[i].InAdcValue = AdcDmaAverageBuffer[i];
		}
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	static uint8_t AdcCount;
	uint8_t i;

	if (AdcCount < 5U)
	{
		AdcCount++;

		for (i = 0U; i < kInputCount; i++)
		{
			AdcDmaAcc[i] += AdcDmaBuffer[i];
		}
	}
	else
	{

		for (i = 0U; i < kInputCount; i++)
		{
			AdcDmaAverageBuffer[i] = (uint32_t)(AdcDmaAcc[i] / AdcCount);
			AdcDmaAcc[i] = 0U;
		}
		AdcCount = 0U;
		//InputsPeriodic();
	}
}
