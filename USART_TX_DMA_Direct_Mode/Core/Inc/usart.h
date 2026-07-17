/*
 * usart.h
 *
 *  Created on: 27-May-2026
 *      Author: HP
 */

#ifndef INC_USART_H_
#define INC_USART_H_

#define RX_BUFFER_SIZE 64

void USART2_Init(void);
void USART2_SendString(char *s);
void USART2_SendByte(char s);
char USART2_ReceiveByte(void);
uint8_t USART2_ReadByte(void);
void USART2_DMA_Init(void);
void USART2_DMA_SendString(char *str);
void USART2_DMA_SendByte(uint8_t c);


#endif /* INC_USART_H_ */
