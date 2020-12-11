/*
 * wh0802a.c
 *
 *  Created on: Apr 6, 2020
 *      Author: gaschmidt1
 *  Modified on: Ago 04, 2020
 *  	Author: felipe.wormmat
 */
#include <wh0802a.h>

/*----------------------------------- Low Level Functions -------------------------------------------*/

/* sinaliza o barramento */
void wh0802aPutClock(void)
{
	/*Liga DIS_EN*/
	GPIOA->ODR |= (1U << 6U); //2U
	//DelayUs(225);
	/*Desliga DIS_EN*/
	GPIOA->ODR &= ~(1U << 6U); //2U
	DelayUs(100);
}

/* envia um nible no barramento */
void wh0802aPutNibble(uint8_t Nibble)
{
	uint32_t Aux = (uint32_t)Nibble;

	/*Desliga Dis_D4 Dis_D5 Dis_D6 Dis_D7
	 * Lembrar de xingar o botoluz 1x por dia.
	 *
	 * D4 - PortB1
	 * D5 - PortA7
	 * D6 - PortF0
	 * D7 - PortF1
	 * */
	GPIOB->ODR &= ~(0x00000002); //D4
	GPIOA->ODR &= ~(0x00000680); //D5 D6 D7
	/*escreve em D4 D5 D6 D7 o Nibble*/
	GPIOB->ODR |= ((Aux << 1U) & (0x00000002));
	GPIOA->ODR |= ((Aux << 6U) & (0x00000080));
	GPIOA->ODR |= ((Aux << 7U) & (0x00000600));
}

/* envia um byte via barramento de 4 bits */
void wh0802aPutByte(uint8_t Byte)
{
	wh0802aPutNibble((Byte & 0xF0) >> 4U);
	wh0802aPutClock();
	wh0802aPutNibble((Byte & 0x0F) >> 0U);
	wh0802aPutClock();
}

/* envia um dado ao display */
void wh0802aPutData(uint8_t Data)
{
	/*Liga MCU_DIS_Rs*/
	GPIOA->ODR |= (1U << 5U); //3U

	wh0802aPutByte(Data);
}

/* envia uma instrução ao display */
void wh0802aPutCommand(uint8_t Command)
{
	/*Desliga MCU_DIS_Rs*/
	GPIOA->ODR &= ~(1U << 5U); //3U

	wh0802aPutByte(Command);
}

/*------------------------------------ Control Functions -------------------------------------------*/
/* inicia o display */
void wh0802aInitDisplay(void)
{
	//HAL_Delay(1000);
	//	uint8_t VetInit[9] = {0x03, 0x03, 0x03, 0x02, 0x28, 0x08, 0x0C, 0x01, 0x06};
	wh0802aPutNibble(0x03);
	wh0802aPutClock();
	HAL_Delay(1);
	wh0802aPutNibble(0x03);
	wh0802aPutClock();
	HAL_Delay(1);
	wh0802aPutNibble(0x03);
	wh0802aPutClock();
	HAL_Delay(1);
	wh0802aPutNibble(0x02);
	wh0802aPutClock();
	HAL_Delay(1);

	wh0802aPutCommand(0x28);
	HAL_Delay(1);
	wh0802aPutCommand(0x08);
	HAL_Delay(1);
	wh0802aPutCommand(0x0F);
	HAL_Delay(1);
	wh0802aPutCommand(0x06);
	HAL_Delay(1);
	wh0802aPutCommand(0x02);
	HAL_Delay(2);
}
/* coloca o display em modo texto */
void wh0802aSetTextMode(void)
{
	wh0802aPutCommand(FUNCTION_SET | DATA_LENGTH_DL);
}

/* limpa a tela em modo texto */
void wh0802aClearScreen(void)
{
	//wh0802aPutCommand(FUNCTION_SET | DATA_LENGTH_DL);
	wh0802aPutCommand(DISPLAY_CLEAR);
	DelayUs(3000U);
}

/* traz o cursor para 0,0 em modo texto */
void wh0802aReturnHome(void)
{
	//wh0802aPutCommand(FUNCTION_SET | DATA_LENGTH_DL);
	wh0802aPutCommand(RETURN_HOME);
}

/* coloca o display em modo de economia de energia */
void wh0802aStandby(void)
{
	wh0802aPutCommand(EXTENDED_FUNCTION_SET | DATA_LENGTH_DL | EXTENDED_INSTRUCTION_RE);
	wh0802aPutCommand(STANDBY);
}

/* vai para determinada posição em modo texto */
void wh0802aGoToLC(uint8_t Lin, uint8_t Col)
{
	switch (Lin)
	{
	case 1:
		wh0802aPutCommand((0x80 - 1U) + Col);
		break;
	case 2:
		wh0802aPutCommand((0xC0 - 1U) + Col);
		break;
	case 3:
		wh0802aPutCommand((0xA0 - 1U) + Col);
		break;
	case 4:
		wh0802aPutCommand((0xB0 - 1U) + Col);
		break;
	default:
		wh0802aPutCommand((0x80 - 1U) + Col);
		break;
	}
}
/*--------------------------------------- Text Functions -------------------------------------------*/
/* escreve uma string de caracteres em modo texto */
void wh0802aPutString(uint8_t *ptr, uint8_t length)
{
	int i = 0U;

	for (i = 0U; i < length; i++)
	{
		wh0802aPutData(ptr[i]);
		DelayUs(150);
	}
}

/* escreve um caracter em modo texto */
void wh0802aPutChar(uint8_t inpChr)
{
	wh0802aPutData(inpChr);
	DelayUs(150);
}

/*--------------------------------------Graphic Functions-------------------------------------------*/
#ifdef Graphics
uint8_t RamDisplay[18U * 32U];

/*Mapa de caracteres da tabela ASCII*/
uint8_t AscIITable[495] = {
	0x00, 0x00, 0x00, 0x00, 0x00, //// Espaço
	0x00, 0x00, 0x5F, 0x00, 0x00, //// !
	0x00, 0x07, 0x00, 0x07, 0x00, //// #
	0x14, 0x7F, 0x14, 0x7F, 0x14, //// "
	0x24, 0x2A, 0x7F, 0x2A, 0x12, //// $
	0x23, 0x13, 0x08, 0x64, 0x62, //// %
	0x36, 0x49, 0x55, 0x22, 0x50, //// &
	0x00, 0x05, 0x03, 0x00, 0x00, //// '
	0x00, 0x1C, 0x22, 0x41, 0x00, //// (
	0x00, 0x41, 0x22, 0x1C, 0x00, //// )
	0x08, 0x2A, 0x1C, 0x2A, 0x08, //// *
	0x08, 0x08, 0x3E, 0x08, 0x08, //// +
	0x00, 0x50, 0x30, 0x00, 0x00, //// ,
	0x08, 0x08, 0x08, 0x08, 0x08, //// -
	0x00, 0x30, 0x30, 0x00, 0x00, //// .
	0x20, 0x10, 0x08, 0x04, 0x02, //// /
	0x3E, 0x51, 0x49, 0x45, 0x3E, //// 0
	0x00, 0x42, 0x7F, 0x40, 0x00, //// 1
	0x42, 0x61, 0x51, 0x49, 0x46, //// 2
	0x21, 0x41, 0x45, 0x4B, 0x31, //// 3
	0x18, 0x14, 0x12, 0x7F, 0x10, //// 4
	0x27, 0x45, 0x45, 0x45, 0x39, //// 5
	0x3C, 0x4A, 0x49, 0x49, 0x30, //// 6
	0x01, 0x71, 0x09, 0x05, 0x03, //// 7
	0x36, 0x49, 0x49, 0x49, 0x36, //// 8
	0x06, 0x49, 0x49, 0x29, 0x1E, //// 9
	0x00, 0x36, 0x36, 0x00, 0x00, //// :
	0x00, 0x56, 0x36, 0x00, 0x00, //// ,
	0x00, 0x08, 0x14, 0x22, 0x41, //// <
	0x14, 0x14, 0x14, 0x14, 0x14, //// =
	0x41, 0x22, 0x14, 0x08, 0x00, //// >
	0x02, 0x01, 0x51, 0x09, 0x06, //// ?
	0x32, 0x49, 0x79, 0x41, 0x3E, //// @
	0x7E, 0x11, 0x11, 0x11, 0x7E, //// A
	0x7F, 0x49, 0x49, 0x49, 0x36, //// B
	0x3E, 0x41, 0x41, 0x41, 0x22, //// C
	0x7F, 0x41, 0x41, 0x22, 0x1C, //// D
	0x7F, 0x49, 0x49, 0x49, 0x41, //// E
	0x7F, 0x09, 0x09, 0x01, 0x01, //// F
	0x3E, 0x41, 0x41, 0x51, 0x32, //// G
	0x7F, 0x08, 0x08, 0x08, 0x7F, //// H
	0x00, 0x41, 0x7F, 0x41, 0x00, //// I
	0x20, 0x40, 0x41, 0x3F, 0x01, //// J
	0x7F, 0x08, 0x14, 0x22, 0x41, //// K
	0x7F, 0x40, 0x40, 0x40, 0x40, //// L
	0x7F, 0x02, 0x04, 0x02, 0x7F, //// M
	0x7F, 0x04, 0x08, 0x10, 0x7F, //// N
	0x3E, 0x41, 0x41, 0x41, 0x3E, //// O
	0x7F, 0x09, 0x09, 0x09, 0x06, //// P
	0x3E, 0x41, 0x51, 0x21, 0x5E, //// Q
	0x7F, 0x09, 0x19, 0x29, 0x46, //// R
	0x46, 0x49, 0x49, 0x49, 0x31, //// S
	0x01, 0x01, 0x7F, 0x01, 0x01, //// T
	0x3F, 0x40, 0x40, 0x40, 0x3F, //// U
	0x1F, 0x20, 0x40, 0x20, 0x1F, //// V
	0x7F, 0x20, 0x18, 0x20, 0x7F, //// W
	0x63, 0x14, 0x08, 0x14, 0x63, //// X
	0x03, 0x04, 0x78, 0x04, 0x03, //// Y
	0x61, 0x51, 0x49, 0x45, 0x43, //// Z
	0x00, 0x00, 0x7F, 0x41, 0x41, //// '['
	0x02, 0x04, 0x08, 0x10, 0x20, //// '\'
	0x41, 0x41, 0x7F, 0x00, 0x00, //// ']'
	0x04, 0x02, 0x01, 0x02, 0x04, //// '^'
	0x40, 0x40, 0x40, 0x40, 0x40, //// '_'
	0x00, 0x01, 0x02, 0x04, 0x00, //// '`'
	0x20, 0x54, 0x54, 0x54, 0x78, //// a
	0x7F, 0x48, 0x44, 0x44, 0x38, //// b
	0x38, 0x44, 0x44, 0x44, 0x20, //// c
	0x38, 0x44, 0x44, 0x48, 0x7F, //// d
	0x38, 0x54, 0x54, 0x54, 0x18, //// e
	0x08, 0x7E, 0x09, 0x01, 0x02, //// f
	0x08, 0x14, 0x54, 0x54, 0x3C, //// g
	0x7F, 0x08, 0x04, 0x04, 0x78, //// h
	0x40, 0x44, 0x7D, 0x40, 0x00, //// i
	0x20, 0x40, 0x44, 0x3D, 0x00, //// j
	0x00, 0x7F, 0x10, 0x28, 0x44, //// k
	0x00, 0x41, 0x7F, 0x40, 0x00, //// l
	0x7C, 0x04, 0x18, 0x04, 0x78, //// m
	0x7C, 0x08, 0x04, 0x04, 0x78, //// n
	0x38, 0x44, 0x44, 0x44, 0x38, //// o
	0x7C, 0x14, 0x14, 0x14, 0x08, //// p
	0x08, 0x14, 0x14, 0x18, 0x7C, //// q
	0x7C, 0x08, 0x04, 0x04, 0x08, //// r
	0x48, 0x54, 0x54, 0x54, 0x20, //// s
	0x04, 0x3F, 0x44, 0x40, 0x20, //// t
	0x3C, 0x40, 0x40, 0x20, 0x7C, //// u
	0x1C, 0x20, 0x40, 0x20, 0x1C, //// v
	0x3C, 0x40, 0x30, 0x40, 0x3C, //// w
	0x44, 0x28, 0x10, 0x28, 0x44, //// x
	0x0C, 0x50, 0x50, 0x50, 0x3C, //// y
	0x44, 0x64, 0x54, 0x4C, 0x44, //// z
	0x00, 0x08, 0x36, 0x41, 0x00, //// '{'
	0x00, 0x00, 0x7F, 0x00, 0x00, //// '|'
	0x00, 0x41, 0x36, 0x08, 0x00, //// '}'
};
#endif
/* coloca o display em modo grafico*/
void wh0802aSetGraphicsMode(void)
{
	wh0802aPutCommand(EXTENDED_FUNCTION_SET | DATA_LENGTH_DL);
	wh0802aPutCommand(EXTENDED_FUNCTION_SET | DATA_LENGTH_DL | EXTENDED_INSTRUCTION_RE);
	wh0802aPutCommand(EXTENDED_FUNCTION_SET | DATA_LENGTH_DL | EXTENDED_INSTRUCTION_RE | GRAPHIC_ON_G);
}
#ifdef Graphics
/* descarrega um bufer pre configurado na tela, o buffer deve ter o tamanho da tela */
void wh0802aRefresh(uint8_t *Ram)
{
	uint8_t j, k;

	for (j = 0U; j < 32U; j++)
	{
		wh0802aPutCommand(SET_GRAPHIC_RAM_ADDRESS | j);
		wh0802aPutCommand(SET_GRAPHIC_RAM_ADDRESS);

		for (k = 0; k < 18U; k++)
		{
			wh0802aPutData(*Ram++);
		}
	}
}

/*--------------------------------------High Level Functions----------------------------------------*/
/* desenha um pixel no display no ponto (x,y) Color pode ser 1 pixel ligado ou 0 pixel desligado*/
void GLcdPutPixel(uint8_t PosX, uint8_t PosY, uint8_t Color, uint8_t *Ram)
{
	if (Color)
	{
		Ram[(PosX / 8) + (PosY * (144U / 8U))] |= (0x80 >> (PosX % 8));
	}
	else
	{
		Ram[(PosX / 8) + (PosY * (144U / 8U))] &= ~(0x80 >> (PosX % 8));
	}
}

/* escreve um texto no display na posiçao (x,y) superior esquerdo --textptr pode ser uma constante string ou o ponteiro de uma string --size pode ser 1 para caracter 5x7 e 2 para 10x14
--color pode ser 1 para pixel ligado e 0 para pixel desligado*/
void GLcdPutText(uint8_t x, uint8_t y, uint8_t *TextPtr, uint8_t Size, uint8_t Color, uint8_t *Ram)
{
	uint8_t i, j, k, l, m, n;
	uint8_t PixelData[5U];

	for (i = 0U; TextPtr[i] != '\0'; ++i, ++x)
	{

		for (n = 0U; n < 5U; n++)
		{
			PixelData[1] = AscIITable[5U * TextPtr[i] - 32U * 5U + n];
		}

		if (x + 5U * Size >= 144U)
		{
			x = 0U;
			y += 7U * Size + 1U;
		}

		for (j = 0U; j < 5U; ++j, x += Size)
		{
			for (k = 0U; k < 7U * Size; ++k)
			{
				if (PixelData[j] & (1U << k))
				{
					for (l = 0U; l < Size; ++l)
					{
						for (m = 0U; m < Size; ++m)
						{
							GLcdPutPixel(x + m, y + k * Size + l, Color, (uint8_t *)Ram);
						}
					}
				}
			}
		}
	}
}

/* desenha uma linha no display usando o algoritino Bresenham's entre os pontos (x1,y1) e (x2,y2) Color pode ser 1 pixel ligado ou 0 pixel desligado*/
void GLcdPutLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t Color, uint8_t *Ram)
{
	uint8_t x, y, dx, dy;
	int8_t addX, addY;
	int16_t P;
	uint8_t i;

	if (x2 > x1)
		dx = x2 - x1;
	else
		dx = x1 - x2;

	if (y2 > y1)
		dy = y2 - y1;
	else
		dy = y1 - y2;

	x = x1;
	y = y1;

	if (x1 > x2)
		addX = -1;
	else
		addX = 1;

	if (y1 > y2)
		addY = -1;
	else
		addY = 1;

	if (dx >= dy)
	{
		P = 2 * dy - dx;

		for (i = 0; i <= dx; ++i)
		{
			GLcdPutPixel(x, y, Color, (uint8_t *)Ram);

			if (P < 0)
			{
				P += 2 * dy;
				x += addX;
			}
			else
			{
				P += 2 * dy - 2 * dx;
				x += addX;
				y += addY;
			}
		}
	}
	else
	{
		P = 2 * dx - dy;

		for (i = 0; i <= dy; ++i)
		{
			GLcdPutPixel(x, y, Color, (uint8_t *)Ram);

			if (P < 0)
			{
				P += 2 * dx;
				y += addY;
			}
			else
			{
				P += 2 * dx - 2 * dy;
				x += addX;
				y += addY;
			}
		}
	}
}

/* desenha um retangulo no display entre dois pontos Color pode ser 1 pixel ligado ou 0 pixel desligado fill diz se o retangulo é sólido ou vazado*/
void GLcdPutRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t Fill, uint8_t Color, uint8_t *Ram)
{
	if (Fill)
	{
		uint8_t y, yMax;
		if (y1 < y2)
		{
			y = y1;
			yMax = y2;
		}
		else
		{
			y = y2;
			yMax = y1;
		}

		for (; y <= yMax; ++y)
		{
			GLcdPutLine(x1, y, x2, y, Color, (uint8_t *)Ram);
		}
	}
	else
	{
		GLcdPutLine(x1, y1, x2, y1, Color, (uint8_t *)Ram);
		GLcdPutLine(x1, y2, x2, y2, Color, (uint8_t *)Ram);
		GLcdPutLine(x1, y1, x1, y2, Color, (uint8_t *)Ram);
		GLcdPutLine(x2, y1, x2, y2, Color, (uint8_t *)Ram);
	}
}

/* desenha um retangulo no display usando um ponto (x, y) e um raio (Radius) fill diz se o retangulo é sólido ou vazado*/
void GLcdPutCircle(uint8_t x, uint8_t y, uint8_t Radius, uint8_t Fill, uint8_t Color, uint8_t *Ram)
{
	int8_t a, b, P;

	a = 0U;
	b = Radius;
	P = 1U - Radius;

	do
	{
		if (Fill)
		{
			GLcdPutLine(x - a, y + b, x + a, y + b, Color, (uint8_t *)Ram);
			GLcdPutLine(x - a, y - b, x + a, y - b, Color, (uint8_t *)Ram);
			GLcdPutLine(x - b, y + a, x + b, y + a, Color, (uint8_t *)Ram);
			GLcdPutLine(x - b, y - a, x + b, y - a, Color, (uint8_t *)Ram);
		}
		else
		{
			GLcdPutPixel(a + x, b + y, Color, (uint8_t *)Ram);
			GLcdPutPixel(b + x, a + y, Color, (uint8_t *)Ram);
			GLcdPutPixel(x - a, b + y, Color, (uint8_t *)Ram);
			GLcdPutPixel(x - b, a + y, Color, (uint8_t *)Ram);
			GLcdPutPixel(b + x, y - a, Color, (uint8_t *)Ram);
			GLcdPutPixel(a + x, y - b, Color, (uint8_t *)Ram);
			GLcdPutPixel(x - a, y - b, Color, (uint8_t *)Ram);
			GLcdPutPixel(x - b, y - a, Color, (uint8_t *)Ram);
		}

		if (P < 0)
		{
			P += 3U + 2U * a++;
		}
		else
		{
			P += 5U + 2U * (a++ - b--);
		}
	} while (a <= b);
}

/*Plota um bitmap no display na posição (x,y) --SizeX é a largura em pixels do bitmap --Sizey é a altura em pixels do bitmap --Bitmap é um ponteiro para o vetor com o bitmap*/
void GLcdPutBitMap(uint8_t x, uint8_t y, uint8_t *BitMap, uint8_t SizeX, uint8_t SizeY, uint8_t *Ram)
{
	uint8_t px, py, cnt = 0U;
	int8_t Pixels, Lines;
	uint8_t *Ptr;

	Ptr = BitMap;

	Lines = SizeY;

	do
	{

		for (px = 0U; px < SizeX; px++)
		{

			if (((Lines - 8U) >= 8U) || ((Lines - 8U) == 0U))
			{
				Pixels = 8U;
			}
			else
			{
				Pixels = Lines;
			}

			for (py = 0U; py < Pixels; py++)
			{

				if (*(Ptr + px + SizeX * cnt) & (0x01 << py))
				{
					GLcdPutPixel((x + px), (y + py + (8U * cnt)), 1, (uint8_t *)Ram);
				}
			}
		}
		Lines -= 8U;
		cnt++;

	} while (Lines > 0U);
}
#endif
