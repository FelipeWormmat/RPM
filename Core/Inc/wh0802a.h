/*
 * wh0802a.h
 *
 *  Created on: Apr 6, 2020
 *      Author: gaschmidt1
 *  Modified on: Ago 04, 2020
 *  	Author: Felipe Wormmat
 */

#ifndef SRC_WH0802A_H_
#define SRC_WH0802A_H_

#include "main.h"

// Instruction Set 1: (RE=0: Basic Instruction)
#define DISPLAY_CLEAR 0x01          // Fill DDRAM with "20H" and set DDRAM address counter (AC) to "00H"
#define RETURN_HOME 0x02            // Set DDRAM address counter (AC) to "00H", and put cursor
#define ENTRY_MODE_SET 0x04         // Set cursor position and display shift when doing write or read
#define DISPLAY_CONTROL 0x08        // D=1: Display ON, C=1: Cursor ON, B=1: Character Blink ON
#define CURSOR_DISPLAY_CONTROL 0x10 // Cursor position and display shift control; the content of
#define FUNCTION_SET 0x20           // DL=1 8-bit interface, DL=0 4-bit interface
#define SET_CGRAM_ADDRESS 0x40      // Set CGRAM address to address counter (AC)
#define SET_DDRAM_ADDRESS 0x80      // Set DDRAM address to address counter (AC)
// AC6 is fixed to 0

// Instruction set 2: (RE=1: extended instruction)
#define STANDBY 0x01                 // Enter standby mode, any other instruction can terminate.
#define SCROLL_OR_RAM_ADDR_SEL 0x02  // SR=1: enable vertical scroll position
#define REVERSE_BY_LINE 0x04         // Select 1 out of 4 line (in DDRAM) and decide whether to
#define EXTENDED_FUNCTION_SET 0x20   // DL=1 :8-bit interface, DL=0 :4-bit interface
#define SET_SCROLL_ADDRESS 0x40      // G=1 :graphic display ON, G=0 :graphic display OFF
#define SET_GRAPHIC_RAM_ADDRESS 0x80 // Set GDRAM address to address counter (AC)

// Parameters regarding Instruction Sets 1 & 2
#define DISPLAY_SHIFT_S 0x01         // Set 1, ENTRY_MODE_SET
#define INCREASE_DECREASE_ID 0x02    // Set 1, ENTRY_MODE_SET
#define CURSOR_BLINK_ON_B 0x01       // Set 1, DISPLAY_CONTROL
#define CURSOR_ON_C 0x02             // Set 1, DISPLAY_CONTROL
#define DISPLAY_ON_D 0x04            // Set 1, DISPLAY_CONTROL
#define SHIFT_RL 0x04                // Set 1, CURSOR_DISPLAY_CONTROL
#define CURSOR_SC 0x08               // Set 1, CURSOR_DISPLAY_CONTROL
#define EXTENDED_INSTRUCTION_RE 0x04 // Set 1, FUNCTION_SET; Set 2, EXTENDED_FUNTION_SET
#define DATA_LENGTH_DL 0x00          // Set 1, FUNCTION_SET; Set 2, EXTENDED_FUNTION_SET
#define REVERSE_BY_LINE_R0 0x01      // Set 2, REVERSE_BY_LINE
#define REVERSE_BY_LINE_R1 0x02      // Set 2, REVERSE_BY_LINE
#define EN_VERTICAL_SCROLL_SR 0x01   // Set 2, SCROLL_OR_RAM_ADDR_SEL
#define GRAPHIC_ON_G 0x02            // Set 2, EXTENDED_FUNTION_SET

#define BUSY_FLAG_BF 0x80

#define st7029Clear 0x01;
#define st7029Home 0x02;
#define st7029Mode 0x06;
#define st7029Status 0x0C;
#define st7029Shift 0x14;
#define st70294Bit 0x32;
#define st70294Extend 0x34;

void wh0802aPutClock(void);

void wh0802aPutNibble(uint8_t Nibble);

void wh0802aPutByte(uint8_t Byte);

void wh0802aPutData(uint8_t Data);

void wh0802aPutCommand(uint8_t Command);

void wh0802aInitDisplay(void);

void wh0802aSetTextMode(void);

void wh0802aClearScreen(void);

void wh0802aReturnHome(void);

void wh0802aStandby(void);

void wh0802aGoToLC(uint8_t Lin, uint8_t Col);

void wh0802aPutString(uint8_t *ptr, uint8_t length);

void wh0802aPutChar(uint8_t inpChr);
/*--------------------------------------------------Graficos-------------------------------------------*/

void wh0802aSetGraphicsMode(void);
#ifdef Graphics
void wh0802aRefresh(uint8_t *RamDisplay);

void GLcdPutPixelPutPixel(uint8_t PosX, uint8_t PosY, uint8_t Color, uint8_t *Ram);

void GLcdPutText(uint8_t x, uint8_t y, uint8_t *TextPtr, uint8_t Size, uint8_t Color, uint8_t *Ram);

void GLcdPutLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t Color, uint8_t *Ram);

void GLcdPutRect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t Fill, uint8_t Color, uint8_t *Ram);

void GLcdPutCircle(uint8_t x, uint8_t y, uint8_t Radius, uint8_t Fill, uint8_t Color, uint8_t *Ram);

void GLcdPutBitMap(uint8_t x, uint8_t y, uint8_t *BitMap, uint8_t SizeX, uint8_t SizeY, uint8_t *Ram);

#endif

#endif /* SRC_WH0802A_H_ */
