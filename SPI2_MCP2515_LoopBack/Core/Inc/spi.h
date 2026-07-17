/*
 * spi.h
 *
 *  Created on: 09-Jun-2026
 *      Author: HP
 */

#include "stm32f407xx.h"


void SPI2_GPIO_Init(void);
void SPI2_Init(void);
uint8_t SPI2_TransferByte(uint8_t byte);
void SPI2_TransmitBuffer(const uint8_t *tx_buf, uint16_t len);
void SPI2_Test(void);
void SPI2_ReceiveBuffer(uint8_t *rx_buf, uint16_t len);
void SPI2_TransferBuffer(const uint8_t *tx_buf,
                         uint8_t *rx_buf,
                         uint16_t len);
void SPI2_WaitNotBusy(void);


#define MCP2515_CS_LOW()   (GPIOB->BSRR = GPIO_BSRR_BR12)
#define MCP2515_CS_HIGH()  (GPIOB->BSRR = GPIO_BSRR_BS12)

#define MAX7219_CS_LOW()   (GPIOB->BSRR = GPIO_BSRR_BR9)
#define MAX7219_CS_HIGH()  (GPIOB->BSRR = GPIO_BSRR_BS9)
