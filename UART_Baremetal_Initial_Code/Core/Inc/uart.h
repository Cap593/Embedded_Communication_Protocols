/*
 * uart.h
 *
 *
 *      Author: Suhas Kokkadan
 */

#ifndef INC_UART_H_
#define INC_UART_H_


void USART2_Init(void);
void USART2_SendString(char *s);
void USART2_SendByte(char s);
char USART2_ReceiveByte(void);



#endif /* INC_UART_H_ */
