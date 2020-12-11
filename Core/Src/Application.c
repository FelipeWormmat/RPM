/*
 * Application.c
 *
 *  Created on: Jul 27, 2020
 *      Author: Felipe Wormmat
 */

/* ------------------------------------- --------------------------------------
 * TODO List Horimetro 3.0
 *
 * Fazer	Testar	Descrição
 * OK		OK		- Fazer bibliotca do Display
 * OK		OK		- Fazer leitura das entradas
 * 					- Fazer biblioteca do RTC
 * OK		OK		- Fazer aplicação HORIMETRO
 * OK		OK		- Fazer aplicação HORIMETRO + CICLOS
 * OK		OK		- Fazer aplicação HORIMETRO + TENSÃO
 * OK		OK		- Fazer aplicação HORIMETRO + %TENSÃO
 * OK		OK		- Fazer aplicação ALL APLICATIONS
 * 					- Fazer debug por serial
 * 					- Fazer Leitura de qual aplicação via serial
 * 					- Colocar o WDG
 * OK		OK		- Função de salvar na flash quando estiver faltando energia
 *					- Teste de tempo de gravaçõa
 * OK		OK		- Teste de Gravação no final da flash
 * OK		OK		- teste de gravação em troca de pagina
 * OK		OK		- teste de gravação no inico da flash
 *					- teste de inicialização com energia baixa
 *					- teste de queda de tensão
 *
 * */

/* Alteração/Criação de novas aplicações.
 *
 * ApplicationType:
 * 		Enum de aplicações, definir nome da nova aplicação.
 *
 * Horimetro.AppType:
 * 		Setar nova aplicação para o horimetro.
 *
 * Criar funsão para a nova aplicação.
 *
 * ApplicationTask:
 * 		Criar novo "case" e chamar a funsão da nova aplicação.
 * */

/*TODO Tabela de erros
 *
 * Flaha 8 - Atingiu o maximo de ciclos
 * Falha 7 - Atingiu o maximo de horas
 * Falha 6 - Tensao baixa
 *
 * */

/* Private includes ----------------------------------------------------------*/
#include <Application.h>
#include <Input.h>
#include <eeprom.h>
#include <wh0802a.h>
/* Private typedef -----------------------------------------------------------*/
typedef enum
{
	eAppHorimetro = 0U,
	eAppHorimetroCycles = 1U,
	eAppHorimetroPercentVoutage = 2U,
	eAppHorimetroVoutage = 3U,
	eAppHorimetroAll = 4U,
	eAppHorimetroRpm = 5U

} ApplicationType;

enum EepromVirtAdd
{
	eVirtAddrHourHigh = 0x0001U,
	eVirtAddrHourLow = 0x0002U,
	eVirtAddrMinuteSecond = 0x0003U,
	eVirtAddrCicleHigh = 0x0004U,
	eVirtAddrCicleLow = 0x0005U,
	eVirtAddrApp = 0x0006U,
};

enum VetorVirtAdd
{
	eVetAddrHourHigh = 0U,
	eVetAddrHourLow = 1U,
	eVetAddrMinuteSecond = 2U,
	eVetAddrCicleHigh = 3U,
	eVetAddrCicleLow = 4U,
	eVetAddrApp = 5U,
};

typedef struct tmr
{
	volatile uint16_t msTmrCount;
	volatile uint8_t msTmrFlag;

	uint8_t sTmrCount;
	uint8_t sTmrFlag;

	uint8_t mTmrCount;
	uint8_t mTmrFlag;

	uint32_t hTmrCount;
	uint8_t hTmrFlag;

	uint8_t TmrAppTaskCount;
	uint8_t TmrAppTaskFlag;
} TMR;

typedef struct horimetroType
{
	TMR Timer;
	uint32_t Cycles;
	ApplicationType AppType;

	uint8_t FlagLowEnergy;
	uint8_t FlagInputCycles;
	uint8_t FlagInputReset;

} HorimetroType;

HorimetroType Horimetro;

/* Private defines -----------------------------------------------------------*/
#define kStraightSlope (uint32_t)(0.887214 * 1024)
#define kLinearCoefficient (uint32_t)(58.616593 * 1024)
#define DC A0

/* Private variables ---------------------------------------------------------*/
uint8_t Buffer_LCD[NUM_LIN_LCD * NUM_COL_LCD] = {
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', // 8 COLUNAS DA PRIMENIRA LINHA
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '	// 8 COLUNAS DA SEGUNDA LINHA
};
//uint16_t VirtAddVarTab [NB_OF_VAR] = { eVirtAddrHourHigh , eVirtAddrHourLow , eVirtAddrMinute , eVirtAddrCicleHigh , eVirtAddrCicleLow , eVirtAddrApp };
uint16_t VirtAddVarTab[NB_OF_VAR] = {eVirtAddrHourHigh, eVirtAddrHourLow, eVirtAddrMinuteSecond, eVirtAddrCicleHigh, eVirtAddrCicleLow};
uint16_t VarDataTab[NB_OF_VAR] = {0, 0, 0, 0, 0};
uint16_t freq = 0;

/* Private function prototypes -----------------------------------------------*/
void CarregaBuffer_LCD(char *Buffer, uint8_t LinhaBuffer, uint8_t ColunaBuffer, uint32_t Valor);
void ApplicationTimer(void);
void AppReadConfig(void);
/* Private user code ---------------------------------------------------------*/

void AppTimeTask(void)
{
	/*Se entrada para contar hora esta ligada*/
	if (InputsGetState(eInputHour) == GPIO_PIN_SET)
	{
		if (++Horimetro.Timer.msTmrCount >= 1000)
		{ /*Desliga flag zera contador*/
			Horimetro.Timer.msTmrCount = 0;
			/* Passou 1 segundo*/
			Horimetro.Timer.sTmrFlag = true;

			if (++Horimetro.Timer.sTmrCount >= 60)
			{ /*Desliga flag zera contador*/
				Horimetro.Timer.sTmrCount = 0;
				/*Passou 1 minuto*/
				Horimetro.Timer.mTmrFlag = true;

				if (++Horimetro.Timer.mTmrCount >= 60)
				{
					/*Zera contador e liga flag*/
					Horimetro.Timer.mTmrCount = 0;
					/* Passou 1 hora */
					Horimetro.Timer.hTmrFlag = true;
					++Horimetro.Timer.hTmrCount;
				}
			}
		}
	}
}

void ApplicationInit(void)
{
	/*Escolhe qual aplicação rodar*/
	//	Horimetro.AppType	= eAppHorimetroTeste;
	//	Horimetro.AppType 	= eAppHorimetroAll;
	//	Horimetro.AppType 	= eAppHorimetro;
	//	Horimetro.AppType 	= eAppHorimetroCycles;
	//	Horimetro.AppType 	= eAppHorimetroVoutage;
	//	Horimetro.AppType 	= eAppHorimetroPercentVoutage;
	Horimetro.AppType = eAppHorimetroRpm;

	/*Inicialização das variaveis de controle Applicação*/
	Horimetro.Cycles = 0;

	/*Inicialização das variaveis de controle das entradas*/
	Horimetro.FlagInputCycles = false;
	Horimetro.FlagInputReset = false;
	if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET)
		Horimetro.FlagLowEnergy = false;
	else
		Horimetro.FlagLowEnergy = true;

	/*Inicialização das variaveis de controle do TMR*/
	Horimetro.Timer.hTmrCount = 0;
	Horimetro.Timer.mTmrCount = 0;
	Horimetro.Timer.sTmrCount = 0;
	Horimetro.Timer.msTmrCount = 0;
	Horimetro.Timer.hTmrFlag = false;
	Horimetro.Timer.mTmrFlag = false;
	Horimetro.Timer.sTmrFlag = false;
	Horimetro.Timer.msTmrFlag = false;

	/*Inicia EEPROM Emulada*/
	EE_Init();

	/*Le valores da eeprom emulada*/
	AppReadConfig();

	/*Inicia Entradas*/
	InputsInit();

	/*Inicia Display*/
	Atualiza_LCD();
}

void AppHorimetro(void)
{
	CarregaBuffer_LCD("HORA:MIN", 0, 0, 0);

	CarregaBuffer_LCD("%5u", 1, 0, Horimetro.Timer.hTmrCount);
	CarregaBuffer_LCD(":%2u", 1, 5, Horimetro.Timer.mTmrCount);
}

void AppHorimetroCyvles(void)
{
	if (Horimetro.Cycles <= 99999999)
	{
		CarregaBuffer_LCD("%5u", 0, 0, Horimetro.Timer.hTmrCount);
		CarregaBuffer_LCD(":%2u", 0, 5, Horimetro.Timer.mTmrCount);

		/*Teste de tempo*/
		//			CarregaBuffer_LCD("%2u", 0, 0, Horimetro.Timer.hTmrCount);
		//			CarregaBuffer_LCD(":%2u", 0, 2, Horimetro.Timer.mTmrCount);
		//			CarregaBuffer_LCD(":%2u", 0, 5, Horimetro.Timer.sTmrCount);
		/*--------------*/

		CarregaBuffer_LCD("%8u", 1, 0, Horimetro.Cycles);
	}
	else
	{
		CarregaBuffer_LCD("FALHA 8 MAX.CYCL", 0, 0, 0);
	}
}

uint16_t AppGetVoutage()
{
	uint32_t Voutage = (InputsGetValue(eInputA1) * kStraightSlope) + kLinearCoefficient;
	return (Voutage >> 10);
}

void AppHorimetroVoutage(void)
{
	CarregaBuffer_LCD("%5u", 0, 0, Horimetro.Timer.hTmrCount);
	CarregaBuffer_LCD(":%2u", 0, 5, Horimetro.Timer.mTmrCount);

	CarregaBuffer_LCD("%2u", 1, 0, AppGetVoutage() / 100);
	CarregaBuffer_LCD(".%2uV", 1, 2, AppGetVoutage() % 100);
}

void AppHorimetroPercentVoutage()
{
	uint32_t tension;
	CarregaBuffer_LCD("%5u", 0, 0, Horimetro.Timer.hTmrCount);
	CarregaBuffer_LCD(":%2u", 0, 5, Horimetro.Timer.mTmrCount);
	/*Limita acima*/

	tension = AppGetVoutage();
	if (AppGetVoutage() >= 1600)
	{
		tension >>= 1;
	}
	/*Limita abaixo*/
	if (AppGetVoutage() <= 1050)
	{
		CarregaBuffer_LCD("000.00\\%", 1, 0, 0);
	}
	else
	{
		tension = ((tension - 1050) * 454) / 10;

		if (tension > 10000)
			tension = 10000;

		CarregaBuffer_LCD("%3u", 1, 0, tension / 100);
		CarregaBuffer_LCD(".%1u\\%", 1, 3, tension % 100);
	}
}

uint16_t rpm = 0;
uint16_t contagem_zero = 0;
char sensor_ativo = ' A ';

// controle do escravo SPI

uint16_t count = 0;
uint8_t init = 0;

void calcular();
void spi_init_slave();
void zerar();

void zerar()
{
}
uint16_t fgdfgd(void)
{
	uint32_t Rpm = (InputsGetValue(eInputA3) * kStraightSlope) * kLinearCoefficient;
	return (Rpm * 60);
}

uint16_t getFreq(void)
{
	Freq = AppGetFreq();
	if (AppGetFreq() >= 123)
	{
		freq >>= 1;
	}
	/*Limita abaixo*/
	if (AppGetFreq() <= 1050)
	{
		//			CarregaBuffer_LCD("000.00\\%", 1, 0, 0);
	}
	else
	{
			freq = ((freq - 1050);

			if(freq "145")
				freq = 145;

			//			CarregaBuffer_LCD("%3u", 1, 0, tension/100);
			//			CarregaBuffer_LCD(".%1u\\%", 1, 3, tension%100);
	}
}

void AppHorimetroRpm(void)
{
	//	CarregaBuffer_LCD("%5u", 0, 0, Horimetro.Timer.mTmrCount);
	//	CarregaBuffer_LCD(":%2u", 0, 5, Horimetro.Timer.sTmrCount);

	//	CarregaBuffer_LCD("%2u", 1, 0, AppGetRpm()/100);
	//	CarregaBuffer_LCD(".%2uR", 1, 2, AppGetRpm()*1000);
	uint16_t aaa = getFreq();
	CarregaBuffer_LCD("f: %5u", 0, 0, aaa);
}

void AppHorimetroTeste(void)
{
	CarregaBuffer_LCD("%2u", 0, 0, Horimetro.Timer.hTmrCount);
	CarregaBuffer_LCD(":%2u", 0, 2, Horimetro.Timer.mTmrCount);
	CarregaBuffer_LCD(":%2u", 0, 5, Horimetro.Timer.sTmrCount);

	CarregaBuffer_LCD("%2u", 1, 0, AppGetVoutage() / 100);
	CarregaBuffer_LCD(".%2uV", 1, 2, AppGetVoutage() % 100);

	CarregaBuffer_LCD(".%2uRpm", 1, 2, AppGetRpm() * 1000);
	CarregaBuffer_LCD("%1uV", 1, 5, InputsGetState(eInputReset));
	CarregaBuffer_LCD("%1uV", 1, 6, InputsGetState(eInputHour));
	CarregaBuffer_LCD("%1uV", 1, 7, InputsGetState(eInputCycle));
}

void AppReadConfig(void)
{
	IWDG_Refresh();
	for (uint8_t index = 0; index < NB_OF_VAR; index++)
	{
		EE_ReadVariable(VirtAddVarTab[index], &VarDataTab[index]);
	}
	//Horimetro.AppType = VarDataTab[eVetAddrApp]; /*Aplicação foi fixada pois não coube no código a parte de UART para ler a aplicação */
	Horimetro.Timer.sTmrCount = (uint8_t)(0x00FF & VarDataTab[eVetAddrMinuteSecond]);
	Horimetro.Timer.mTmrCount = (uint8_t)(VarDataTab[eVetAddrMinuteSecond] >> 8);
	Horimetro.Timer.hTmrCount = (uint32_t)(((uint32_t)VarDataTab[eVetAddrHourHigh] << 16) | ((uint32_t)VarDataTab[eVetAddrHourLow]));
	Horimetro.Cycles = (uint32_t)(((uint32_t)VarDataTab[eVetAddrCicleHigh] << 16) | ((uint32_t)VarDataTab[eVetAddrCicleLow]));
}

void AppCapture(uint16_t capture)
{
	freq = capture;
	//	CarregaBuffer_LCD("%u", 0, 0, capture);
}

void AppSaveConfig(void)
{
	if (Horimetro.FlagLowEnergy == false)
	{
		//		__disable_irq();

		Horimetro.FlagLowEnergy = true;

		uint8_t index = 0;
		uint16_t VarData[NB_OF_VAR];

		//		for(index = 0; index < 0xFF; index++)
		//		{
		//			/*TESTE PARA VER QUANTO TEMPO RESTA PARA GRAVAÇÃO.*/
		//		}

		/*Atualiza valores na tabela*/
		//		for(uint8_t i=0; i < 50; i++)
		//		{

		VarData[eVetAddrHourHigh] = (uint16_t)(Horimetro.Timer.hTmrCount >> 16);
		VarData[eVetAddrHourLow] = (uint16_t)(0x0000FFFF & Horimetro.Timer.hTmrCount);
		VarData[eVetAddrMinuteSecond] = (Horimetro.Timer.mTmrCount << 8U) | Horimetro.Timer.sTmrCount;
		VarData[eVetAddrCicleHigh] = (uint16_t)(Horimetro.Cycles >> 16);
		VarData[eVetAddrCicleLow] = (uint16_t)(0x0000FFFF & Horimetro.Cycles);
		//VarData[eVetAddrApp] 		= Horimetro.AppType;

		IWDG_Refresh();
		/*Compara valores na EEPROM Emulada*/
		for (index = 0; index < NB_OF_VAR; index++)
		{
			//EE_ReadVariable(VirtAddVarTab[index], &VarData[index]);
			if (index == eVetAddrMinuteSecond)
			{
				if ((VarDataTab[index] & 0xFF00) != (VarData[index] & 0xFF00))
				{
					/*Se diferentes salva*/
					EE_WriteVariable(VirtAddVarTab[index], VarData[index]);
					VarDataTab[index] = VarData[index];
				} /* Se a diferença de segundos for igual aou maior que 10 segundos*/
				else if (((VarData[index] & 0x00FF) - (VarDataTab[index] & 0x00FF)) >= 10U)
				{
					/*Se diferentes salva*/
					EE_WriteVariable(VirtAddVarTab[index], VarData[index]);
					VarDataTab[index] = VarData[index];
				}
			}
			else if (VarDataTab[index] != VarData[index])
			{
				/*Se diferentes salva*/
				EE_WriteVariable(VirtAddVarTab[index], VarData[index]);
				VarDataTab[index] = VarData[index];
			}
		}

		//			Horimetro.Timer.hTmrCount += 0x00010001;
		//			Horimetro.Timer.mTmrCount ++;
		//			Horimetro.Cycles += 0x00010001;
		//		}
		//		__enable_irq();
	}
	else
	{
		Horimetro.FlagLowEnergy = false;
	}
}

void ApplicationTask(void)
{
	static uint16_t appTemp = 0;
	/*TASK DA APLICAÇÃO
	 * 	- Verifica qual apricação é
	 * 		- Horimetro
	 * 		- Horimetro + Ciclos
	 * 		- Horimetro + Tensão
	 * 		- Horimetro + %Tensão
	 * 		- Horimetro + all.
	 * */

	/*TODO Verificar erros no programa*/
	{
		/* Verifica se a Tensão esta OK*/
		if (Horimetro.FlagLowEnergy == false)
		{
			if (Horimetro.Timer.hTmrCount <= 99999)
			{
				switch (Horimetro.AppType)
				{
				case eAppHorimetro:
					AppHorimetro();
					break;
				case eAppHorimetroCycles:
					AppHorimetroCyvles();
					break;
				case eAppHorimetroVoutage:
					AppHorimetroVoutage();
					break;
				case eAppHorimetroPercentVoutage:
					AppHorimetroPercentVoutage();
					break;
				case eAppHorimetroRpm:
					AppHorimetroRpm();
					break;
					//case eAppHorimetroCapture:
					//AppHorimetroCapture();
					break;
				case eAppHorimetroAll:
					if (appTemp < 42) /* Altera a tela a cada 15s (350ms*42 = 15000ms)*/
						AppHorimetro();
					else if (appTemp < 84)
						AppHorimetroCyvles();
					else if (appTemp < 126)
						AppHorimetroVoutage();
					else if (appTemp < 168)
						AppHorimetroPercentVoutage();
					else if (appTemp < 210)
						AppHorimetroRpm();
					else if (appTemp < 252)
						AppHorimetroPercentRpm();
					else
						appTemp = 0;

					appTemp++;
					break;
				default:
					break;
				}
			}
			else
			{
				CarregaBuffer_LCD("FALHA 7 MAX.HORA", 0, 0, 0);
			}
		}
		else
		{
			/*
			 * Salva dados na EEPROM
			 * Escreve tensão baixa no buffer
			 * */
			CarregaBuffer_LCD("E6 BAIXA!TENSAO!", 0, 0, 0);
		}

		/*Verifica botão de reset
		 * */
		if (InputsGetState(eInputReset) == GPIO_PIN_SET && Horimetro.FlagInputReset == false)
		{
			Horimetro.FlagInputReset = true;

			Horimetro.Timer.hTmrCount = 0U;
			Horimetro.Timer.mTmrCount = 0U;
			Horimetro.Timer.sTmrCount = 0U;
			Horimetro.Timer.msTmrCount = 0U;
			Horimetro.Cycles = 0U;

			Horimetro.Timer.hTmrFlag = false;
			Horimetro.Timer.mTmrFlag = false;
			Horimetro.Timer.sTmrFlag = false;
			Horimetro.Timer.msTmrFlag = false;
		}
		else if (InputsGetState(eInputReset) == GPIO_PIN_RESET)
		{
			Horimetro.FlagInputReset = false;
		}

		if (InputsGetState(eInputCycle) == GPIO_PIN_SET && Horimetro.FlagInputCycles == false)
		{
			Horimetro.FlagInputCycles = true;
			Horimetro.Cycles++;
		}
		else if (InputsGetState(eInputCycle) == GPIO_PIN_RESET)
		{
			Horimetro.FlagInputCycles = false;
		}
	}
	{
		/*TODO Escreve erro no display e manda pela serial*/
	}

	Atualiza_LCD();
}

void CarregaBuffer_LCD(char *Buffer, uint8_t LinhaBuffer, uint8_t ColunaBuffer, uint32_t Valor)
{
	uint8_t Digitos = 0;

	if (LinhaBuffer)
		ColunaBuffer += NUM_COL_LCD;

	for (uint8_t Posicao = ColunaBuffer; Posicao < (NUM_LIN_LCD * NUM_COL_LCD); Posicao++)
	{
		if (!(*Buffer))
			break;

		if ((*Buffer) && ((*Buffer) != '%') && ((*Buffer) != '\\')) // SE TEM ALGO PARA ESCREVER
		{
			Buffer_LCD[Posicao] = (*Buffer++); // ESCREVE NO BUFFER
		}
		else if ((*Buffer) && ((*Buffer) == '\\'))
		{
			Buffer++;
			Buffer_LCD[Posicao] = (*Buffer++); // ESCREVE NO BUFFER
		}
		else
		{
			if ((*Buffer) == '%') // SE CONTEUDO = '%'
			{
				Buffer++;
				if (((*Buffer) >= '1') && ((*Buffer) <= '9'))
				{
					Digitos = ((*Buffer) - '0');
					Buffer++;
				}

				if ((*Buffer++) == 'u') // SO SE FOR UM NUMERO SEM SINAL
				{
					if (Digitos > 7) // SE TEM O QUINTO DIGITO
					{
						Buffer_LCD[Posicao] = ((uint8_t)((Valor / 10000000) + '0')); // ENVIA O VALOR DO OITAVO DIGITO
						Posicao++;													 // INCREMENTA O VETOR PARA ESCREVER NA POSICAO CORRETA
					}

					if (Digitos > 6) // SE TEM O QUINTO DIGITO
					{
						Buffer_LCD[Posicao] = ((uint8_t)((Valor % 10000000) / 1000000 + '0')); // ENVIA O VALOR DO SETIMO DIGITO
						Posicao++;															   // INCREMENTA O VETOR PARA ESCREVER NA POSICAO CORRETA
					}

					if (Digitos > 5) // SE TEM O QUINTO DIGITO
					{
						Buffer_LCD[Posicao] = ((uint8_t)((Valor % 1000000) / 100000 + '0')); // ENVIA O VALOR DO SEXTO DIGITO
						Posicao++;															 // INCREMENTA O VETOR PARA ESCREVER NA POSICAO CORRETA
					}

					if (Digitos > 4) // SE TEM O QUINTO DIGITO
					{
						Buffer_LCD[Posicao] = ((uint8_t)((Valor % 100000) / 10000 + '0')); // ENVIA O VALOR DO QUINTO DIGITO
						Posicao++;														   // INCREMENTA O VETOR PARA ESCREVER NA POSICAO CORRETA
					}

					if (Digitos > 3) // SE TEM O QUARTO DIGITO
					{
						Buffer_LCD[Posicao] = ((uint8_t)(((Valor % 10000) / 1000) + '0')); // ENVIA O VALOR DO QUARTO DIGITO
						Posicao++;														   // INCREMENTA O VETOR PARA ESCREVER NA POSICAO CORRETA
					}

					if (Digitos > 2) // SE TEM O TERCEIRO DIGITO
					{
						Buffer_LCD[Posicao] = ((uint8_t)(((Valor % 1000) / 100) + '0')); // ENVIA O VALOR DO TERCEIRO DIGITO
						Posicao++;														 // INCREMENTA O VETOR PARA ESCREVER NA POSICAO CORRETA
					}

					if (Digitos > 1) // SE TEM O SEGUNDO DIGITO
					{
						Buffer_LCD[Posicao] = ((uint8_t)(((Valor % 100) / 10) + '0')); // ENVIA O VALOR DO SEGUNDO DIGITO
						Posicao++;													   // INCREMENTA O VETOR PARA ESCREVER NA POSICAO CORRETA
					}

					if (Digitos)											   // SE TEM O PRIMEIRO DIGITO
						Buffer_LCD[Posicao] = ((uint8_t)((Valor % 10) + '0')); // ENVIA O VALOR DO PRIMEIRO DIGITO
					else
						Posicao--;
				}
				//				else
				//				{
				//					Buffer--;	// ATUALIZA O BUFFER PARA TESTAR O CARACTER CORRETO
				//					Posicao -= 1;	// ATUALIZA O VETOR PARA ESCREVER NA COLUNA CORRETA
				//
				//					if(Digitos)
				//						Digitos = 0;	// ATUALIZA O NUMERO DE DIGITOS
				//
				//					Buffer--;	// ATUALIZA O BUFFER PARA TESTAR O CARACTER CORRETO
				//				}
			}
		}
	}
}

void Atualiza_LCD(void)
{
	static uint8_t Reset_LCD = 99;
	uint8_t Posicao = 0;

	if (++Reset_LCD >= 100)
	{
		Reset_LCD = 0;
		wh0802aInitDisplay(); // CONFIGURA O LCD
	}

	for (Posicao = 0; Posicao < (NUM_LIN_LCD * NUM_COL_LCD); Posicao++)
	{
		if (Posicao == 0)
		{
			wh0802aGoToLC(1, 1); // VAI PARA O COMECO DA PRIMEIRA LINHA
		}
		else if (Posicao == NUM_COL_LCD)
		{
			wh0802aGoToLC(2, 1); // VAI PARA O COMECO DA SEGUNDA  LINHA
		}

		wh0802aPutChar(Buffer_LCD[Posicao]); // ESCREVE ALGO
		Buffer_LCD[Posicao] = (' ');		 // LIMPA O BUFFER
	}
}

/*-------------------------------------------------------------------------
 * 					FUNÇÕES DE TESTE
 * -----------------------------------------------------------------------*/
//void MostraValores()
//{
//	EE_ReadVariable(VirtAddVarTab[0], &VarDataTab[0]);
//	EE_ReadVariable(VirtAddVarTab[1], &VarDataTab[1]);
//	//EE_ReadVariable(VirtAddVarTab[2], &VarDataTab[2]);
//
//	wh0802aClearScreen();
//	//wh0802aPutString((char*)text,(char*)strlen(text));
//	//CarregaBuffer_LCD((uint8_t*)"V1 V2 V3", 0, 0, 0);
//	CarregaBuffer_LCD((uint8_t*)"%5u", 0, 0, VarDataTab[0]);
//	CarregaBuffer_LCD((uint8_t*)"%5u", 1, 0, VarDataTab[1]);
//	//CarregaBuffer_LCD((uint8_t*)"%2u", 1, 6, VarDataTab[2]);
//
//	Atualiza_LCD();
//	HAL_Delay(3000);
//}

//void TesteFlash()
//{
//	/* --- Store successively many values of the three variables in the EEPROM ---*/
//	/* Store 0x1000 values of Variable1 in EEPROM */
//
//
//	/*Funcionamento do teste
//	 * Mostar dados atuais das variaveis
//	 * Grava até mudar de pagina a var 1 var 2 e var 3
//	 * Le dados das var
//	 * grava novamente
//	 * Ve novamente
//	 * */
//
//
//	/*Mostar valores das variaveis por 1000ms	*/
//	MostraValores();
//
//	for (VarValue = 1; VarValue <= 0x1000; VarValue++)
//	{
//		//VarValue = RandNumInt32()%10;
//		EE_WriteVariable(VirtAddVarTab[0], VarValue);
//		//VarValue = RandNumInt32()%10;
//		EE_WriteVariable(VirtAddVarTab[1], VarValue);
//	}
//
//	MostraValores();
//}
