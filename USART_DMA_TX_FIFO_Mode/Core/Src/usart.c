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

    /* Enable RX interrupt, and USART2 interrupt in NVIC*/
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

void USART2_DMA_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    /* Disable stream */
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while(DMA1_Stream6->CR & DMA_SxCR_EN);

    /* Clear old flags */
    DMA1->HIFCR =
          DMA_HIFCR_CTCIF6  |
          DMA_HIFCR_CHTIF6  |
          DMA_HIFCR_CTEIF6  |
          DMA_HIFCR_CDMEIF6 |
          DMA_HIFCR_CFEIF6;

    /* Clear CR */
    DMA1_Stream6->CR = 0;

    /* Channel 4 */
    DMA1_Stream6->CR |=
            (4UL << DMA_SxCR_CHSEL_Pos);

    /* Memory-to-peripheral */
    DMA1_Stream6->CR |= DMA_SxCR_DIR_0;

    /* Memory increment */
    DMA1_Stream6->CR |= DMA_SxCR_MINC;

    /* Peripheral increment disabled */
    DMA1_Stream6->CR &= ~DMA_SxCR_PINC;

    /* Transfer complete interrupt */
    DMA1_Stream6->CR |= DMA_SxCR_TCIE;

    /* FIFO Mode Enabled */
    DMA1_Stream6->FCR |= DMA_SxFCR_DMDIS;

    /* FIFO Threshold - Full FIFO */
    DMA1_Stream6->FCR |= DMA_SxFCR_FTH;

    /* MSIZE - 1 Byte transfer */
    DMA1_Stream6->CR &= ~DMA_SxCR_MSIZE;

    /* PSIZE - 1 Byte transfer */
    DMA1_Stream6->CR &= ~DMA_SxCR_PSIZE;

    /* MBURST - Single beat transfer from memory */
    DMA1_Stream6->CR &= ~DMA_SxCR_MBURST;

    /* PBURST - Single beat transfer from Peripheral */
    DMA1_Stream6->CR &= ~DMA_SxCR_PBURST;

    /* Peripheral address */
    DMA1_Stream6->PAR = (uint32_t)&USART2->DR;

    /* Enable DMA IRQ */
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);

}

void USART2_DMA_SendString(char *str)
{
	uint16_t strln = 0;

	//length of the string
	while(str[strln]){
		strln++;
	}

    /* Disable stream */
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while(DMA1_Stream6->CR & DMA_SxCR_EN);

    /* Clear old flags */
    DMA1->HIFCR =
          DMA_HIFCR_CTCIF6  |
          DMA_HIFCR_CHTIF6  |
          DMA_HIFCR_CTEIF6  |
          DMA_HIFCR_CDMEIF6 |
          DMA_HIFCR_CFEIF6;

    /* Transfer complete interrupt */
    DMA1_Stream6->CR |= DMA_SxCR_TCIE;

    /* configuring to single transfer */
    DMA1_Stream6->CR &= ~DMA_SxCR_MBURST;

    /* Memory address */
    DMA1_Stream6->M0AR = (uint32_t)str;

    /* Number of bytes */
    DMA1_Stream6->NDTR = strln;

    /* Enable DMA stream */
    DMA1_Stream6->CR |= DMA_SxCR_EN;

    /* Enable USART DMA TX */
    USART2->CR3 |= USART_CR3_DMAT;
}

void USART2_DMA_SendByte(uint8_t c)
{
    /* Disable stream */
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while(DMA1_Stream6->CR & DMA_SxCR_EN);

    /* Clear old flags */
    DMA1->HIFCR =
          DMA_HIFCR_CTCIF6  |
          DMA_HIFCR_CHTIF6  |
          DMA_HIFCR_CTEIF6  |
          DMA_HIFCR_CDMEIF6 |
          DMA_HIFCR_CFEIF6;

    /* Transfer complete interrupt */
    DMA1_Stream6->CR |= DMA_SxCR_TCIE;

    /* configuring to single transfer */
    DMA1_Stream6->CR &= ~DMA_SxCR_MBURST;

    /* Memory address */
    DMA1_Stream6->M0AR = (uint32_t)&c;

    /* Number of bytes */
    DMA1_Stream6->NDTR = 1;

    /* Enable DMA stream */
    DMA1_Stream6->CR |= DMA_SxCR_EN;

    /* Enable USART DMA TX */
    USART2->CR3 |= USART_CR3_DMAT;

}

void USART2_DMA_SendBuffer(const char *buffer, uint16_t length)
{
    /* Disable stream */
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while(DMA1_Stream6->CR & DMA_SxCR_EN);

    /* Clear old flags */
    DMA1->HIFCR =
          DMA_HIFCR_CTCIF6  |
          DMA_HIFCR_CHTIF6  |
          DMA_HIFCR_CTEIF6  |
          DMA_HIFCR_CDMEIF6 |
          DMA_HIFCR_CFEIF6;

    /* Transfer complete interrupt */
    DMA1_Stream6->CR |= DMA_SxCR_TCIE;

    /* MBURST - 16 beat transfer from memory */
    DMA1_Stream6->CR |= DMA_SxCR_MBURST;

    /* Memory address */
    DMA1_Stream6->M0AR = (uint32_t)buffer;

    /* Number of bytes */
    DMA1_Stream6->NDTR = length;

    /* Enable DMA stream */
    DMA1_Stream6->CR |= DMA_SxCR_EN;

    /* Enable USART DMA TX */
    USART2->CR3 |= USART_CR3_DMAT;

}

