/*
 * usart.c
 *
 *
 *      Author: Suhas K
 */

#include "stm32f407xx.h"
#include "usart.h"


/* extern variables */
extern volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
extern volatile uint16_t rx_head;
extern volatile uint16_t rx_tail;


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

    /* Enable RX interrupt and USART2 interrupt in NVIC*/
    USART2->CR1 |= USART_CR1_RXNEIE;
    NVIC_EnableIRQ(USART2_IRQn);
}

/* Transmit string from device to terminal */
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

/* Transmit 1 byte from the device to terminal*/
void USART2_SendByte(char c)
{
	while(!(USART2->SR & USART_SR_TXE));
	USART2->DR = c;

	while(!(USART2->SR & USART_SR_TC));
}

/* Receive 1 byte from terminal - polling based */
char USART2_ReceiveByte(void)
{
	while(!(USART2->SR & USART_SR_RXNE));

	return (char)USART2->DR;
}

/* Used for reading characters received via ISR */
uint8_t USART2_ReadByte(void)
{
	uint8_t data;

	// block while buffer is empty
	while(rx_head == rx_tail);

	//read with tail var
	data = rx_buffer[rx_tail];

	//incrementing buffer index
	rx_tail++;

	if(rx_tail >= RX_BUFFER_SIZE)
	{
		// making it circular
		rx_tail = 0;
	}

	return data;
}

