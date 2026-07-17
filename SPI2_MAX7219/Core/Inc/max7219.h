/*
 * max7219.h
 *
 *  Created on: 14-Jun-2026
 *      Author: HP
 */

#ifndef INC_MAX7219_H_
#define INC_MAX7219_H_

#include "stm32f4xx_hal.h"

//register addresses
#define MAX7219_REG_NOOP          0x00
#define MAX7219_REG_DIGIT0        0x01
#define MAX7219_REG_DIGIT1        0x02
#define MAX7219_REG_DIGIT2        0x03
#define MAX7219_REG_DIGIT3        0x04
#define MAX7219_REG_DIGIT4        0x05
#define MAX7219_REG_DIGIT5        0x06
#define MAX7219_REG_DIGIT6        0x07
#define MAX7219_REG_DIGIT7        0x08
#define MAX7219_REG_DECODE_MODE   0x09
#define MAX7219_REG_INTENSITY     0x0A
#define MAX7219_REG_SCAN_LIMIT    0x0B
#define MAX7219_REG_SHUTDOWN      0x0C
#define MAX7219_REG_DISPLAY_TEST  0x0F

void MAX7219_Write(uint8_t address, uint8_t data);
void MAX7219_Init(void);
void MAX7219_Clear(void);
void MAX7219_Fill(void);
void MAX7219_NumberSequenceTest(void);
void MAX7219_DisplayNumber(uint8_t number);
void MAX7219_CheckPosition(void);
void MAX7219_DisplayChar(char ch);
void MAX7219_AlphabetSequenceTest(void);
void MAX7219_CharBlinking(char ch);
void MAX7219_StringBlinking(const char *ch);

const uint8_t* MAX7219_GetCharBitmap(char ch);
void MAX7219_DisplayFrame(uint8_t frame[8]);
uint16_t MAX7219_StrLen(const char *str);
void MAX7219_ScrollText_RtoL(const char *text, uint16_t speed_ms);

#endif /* INC_MAX7219_H_ */
