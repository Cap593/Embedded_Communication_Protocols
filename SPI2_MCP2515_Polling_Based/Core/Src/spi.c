/*
 * spi.c
 *
 *  Created on: 09-Jun-2026
 *      Author: HP
 */

#include "spi.h"

void SPI2_Init(void)
{
    /* Enable SPI2 clock */
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

    /* Disable SPI before configuration */
    SPI2->CR1 &= ~SPI_CR1_SPE;

    /* Clear CR1 configuration bits we care about */
    SPI2->CR1 = 0;

    /*
     * Master mode
     * Software slave management
     * Internal slave select high
     */
    SPI2->CR1 |= SPI_CR1_MSTR;
    SPI2->CR1 |= SPI_CR1_SSM;
    SPI2->CR1 |= SPI_CR1_SSI;

    /*
     * Mode 0: CPOL = 0, CPHA = 0
     */
    SPI2->CR1 &= ~SPI_CR1_CPOL;
    SPI2->CR1 &= ~SPI_CR1_CPHA;

    /*
     * 8-bit frame, MSB first
     */
    SPI2->CR1 &= ~SPI_CR1_DFF;
    SPI2->CR1 &= ~SPI_CR1_LSBFIRST;

    /*
     * Baud rate = PCLK / 32
     * If PCLK1 = 32 MHz, SCK = 1 MHz
     */
    SPI2->CR1 &= ~SPI_CR1_BR;
    SPI2->CR1 |= SPI_CR1_BR_2;

    /*
     * Full duplex 2-line
     */
    SPI2->CR1 &= ~SPI_CR1_BIDIMODE;
    SPI2->CR1 &= ~SPI_CR1_RXONLY;

    /* Enable SPI */
    SPI2->CR1 |= SPI_CR1_SPE;
}

void SPI2_GPIO_Init(void)
{
    /* Enable GPIOB clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /*
     * PB12 = CS GPIO output
     * PB13 = SPI2_SCK AF
     * PB14 = SPI2_MISO AF
     * PB15 = SPI2_MOSI AF
     */

    /* Clear mode bits */
    GPIOB->MODER &= ~((3UL << (12 * 2)) |
                      (3UL << (13 * 2)) |
                      (3UL << (14 * 2)) |
                      (3UL << (15 * 2)));

    /* Set modes */
    GPIOB->MODER |=  ((1UL << (12 * 2)) |   /* PB12 output */
                      (2UL << (13 * 2)) |   /* PB13 AF */
                      (2UL << (14 * 2)) |   /* PB14 AF */
                      (2UL << (15 * 2)));   /* PB15 AF */

    /* Push-pull */
    GPIOB->OTYPER &= ~((1UL << 12) |
                       (1UL << 13) |
                       (1UL << 14) |
                       (1UL << 15));

    /* Speed: medium/high for SPI */
    GPIOB->OSPEEDR &= ~((3UL << (12 * 2)) |
                        (3UL << (13 * 2)) |
                        (3UL << (14 * 2)) |
                        (3UL << (15 * 2)));

    GPIOB->OSPEEDR |=  ((2UL << (12 * 2)) |
                        (2UL << (13 * 2)) |
                        (2UL << (14 * 2)) |
                        (2UL << (15 * 2)));

    /* Pull config: CS pull-up, others no pull */
    GPIOB->PUPDR &= ~((3UL << (12 * 2)) |
                      (3UL << (13 * 2)) |
                      (3UL << (14 * 2)) |
                      (3UL << (15 * 2)));

    GPIOB->PUPDR |=  (1UL << (12 * 2));     /* PB12 pull-up */

    /* AF5 for PB13, PB14, PB15 */
    GPIOB->AFR[1] &= ~((0xFUL << ((13 - 8) * 4)) |
                       (0xFUL << ((14 - 8) * 4)) |
                       (0xFUL << ((15 - 8) * 4)));

    GPIOB->AFR[1] |=  ((5UL << ((13 - 8) * 4)) |
                       (5UL << ((14 - 8) * 4)) |
                       (5UL << ((15 - 8) * 4)));

    /* Keep CS high initially */
    SPI2_CS_HIGH();
}

uint8_t SPI2_TransferByte(uint8_t byte)
{
    uint8_t received;

    //waiting for TX buffer empty
    while(!(SPI2->SR & SPI_SR_TXE));
    *((volatile uint8_t *)&SPI2->DR) = byte;

    //waiting for RX buffer Not empty
    while(!(SPI2->SR & SPI_SR_RXNE));
    received = *((volatile uint8_t *)&SPI2->DR);

    //Wait till SPI completes
    while(SPI2->SR & SPI_SR_BSY);

    return received;
}

void SPI2_TransmitBuffer(const uint8_t *tx_buf, uint16_t len)
{
	//Transmit n number for bytes
    for(uint16_t i = 0; i < len; i++)
    {
    	//only transmitting , ignoring received data
        (void)SPI2_TransferByte(tx_buf[i]);
    }
}

void SPI2_ReceiveBuffer(uint8_t *rx_buf, uint16_t len)
{
	// receive till length
    for(uint16_t i = 0; i < len; i++)
    {
    	// receiving n number of bytes and transmitting dummy data
        rx_buf[i] = SPI2_TransferByte(0xFF);
    }
}

void SPI2_TransferBuffer(const uint8_t *tx_buf,
                         uint8_t *rx_buf,
                         uint16_t len)
{
	// transmit and receive till len variable
    for(uint16_t i = 0; i < len; i++)
    {
        uint8_t tx = 0xFF;

        //if not null address
        if(tx_buf != 0)
        {
            tx = tx_buf[i];
        }

        //transmit and  receive
        uint8_t rx = SPI2_TransferByte(tx);

        //if not null address
        if(rx_buf != 0)
        {
        	//store in reception buffer
            rx_buf[i] = rx;
        }
    }
}

void SPI2_WaitNotBusy(void)
{
	//wait till SPI completes
    while(SPI2->SR & SPI_SR_BSY);
}

void SPI2_Test(void)
{
	uint8_t tx_data[] = {0xAA, 0x55, 0x96, 0x3C};

	SPI2_CS_LOW();
	SPI2_TransmitBuffer(tx_data, sizeof(tx_data));
	SPI2_CS_HIGH();
}
