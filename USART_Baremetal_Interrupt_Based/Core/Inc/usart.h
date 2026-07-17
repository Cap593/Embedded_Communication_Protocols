/*
 * usart.h
 *
 *  Created on: 27-May-2026
 *      Author: HP
 */

#ifndef INC_USART_H_
#define INC_USART_H_

void USART2_Init(void);
void USART2_SendString(char *s);
void USART2_SendByte(char s);
char USART2_ReceiveByte(void);


#endif /* INC_USART_H_ */
