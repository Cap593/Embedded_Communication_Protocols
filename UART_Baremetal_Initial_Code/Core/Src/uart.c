/*
 * uart.c
 *
 *
 *      Author: Suhas Kokkadan
 */

#include "stm32f407xx.h"


void USART2_Init(void)
{
    /* Enable clocks */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;


    /* Configure PA2(TX) and PA3(RX) as Alternate Function */

    GPIOA->MODER &= ~(0x3UL << GPIO_MODER_MODER2_Pos);
    GPIOA->MODER &= ~(0x3UL << GPIO_MODER_MODER3_Pos);

    GPIOA->MODER |=  (0x2UL << GPIO_MODER_MODER2_Pos);
    GPIOA->MODER |=  (0x2UL << GPIO_MODER_MODER3_Pos);


    /* Select AF7 for USART2 */

    GPIOA->AFR[0] &= ~(0xFUL << GPIO_AFRL_AFSEL2_Pos);
    GPIOA->AFR[0] &= ~(0xFUL << GPIO_AFRL_AFSEL3_Pos);

    GPIOA->AFR[0] |=  (0x7UL << GPIO_AFRL_AFSEL2_Pos);
    GPIOA->AFR[0] |=  (0x7UL << GPIO_AFRL_AFSEL3_Pos);


    /* Configure baud rate */

    USART2->BRR = 364;


    /* Configure frame format: 8N1 */

    USART2->CR1 &= ~USART_CR1_M;
    USART2->CR1 &= ~USART_CR1_PCE;

    USART2->CR2 &= ~USART_CR2_STOP;


    /* Enable transmitter and receiver */

    USART2->CR1 |= USART_CR1_TE;
    USART2->CR1 |= USART_CR1_RE;


    /* Enable USART */

    USART2->CR1 |= USART_CR1_UE;
}

void USART2_SendString(char *s)
{
	while(*s != '\0')
	{
		while(!(USART2->SR & USART_SR_TXE));
		USART2->DR = *s++;
	}

	//checking for the last character
	while(!(USART2->SR & USART_SR_TC));
}

void USART2_SendByte(char c)
{
	while(!(USART2->SR & USART_SR_TXE));
	USART2->DR = c;

	while(!(USART2->SR & USART_SR_TC));

	USART2_SendString("\r\n");
}

char USART2_ReceiveByte(void)
{
	while(!(USART2->SR & USART_SR_RXNE));

	return (char)USART2->DR;
}

