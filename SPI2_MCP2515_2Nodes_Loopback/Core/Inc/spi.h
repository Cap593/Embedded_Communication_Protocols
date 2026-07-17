/*
 * spi.h
 *
 *  Created on: 09-Jun-2026
 *      Author: HP
 */

#include "stm32f407xx.h"

typedef enum
{
	MCP2515_NODE_1 = 0,
	MCP2515_NODE_2
}MCP2515_Node_t;

typedef enum
{
    MCP2515_SPI_BUS_1 = 0,
    MCP2515_SPI_BUS_2
} MCP2515_SpiBus_t;

void SPI1_Init(void);
void SPI1_GPIO_Init(void);
uint8_t SPI1_TransferByte(uint8_t data);

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


void MCP2515_SelectNode(MCP2515_Node_t node);
void MCP2515_SelectSpiBus(MCP2515_SpiBus_t bus);


#define MCP2515_CS_LOW()   			(GPIOB->BSRR = GPIO_BSRR_BR12)
#define MCP2515_CS_HIGH()  			(GPIOB->BSRR = GPIO_BSRR_BS12)

#define MAX7219_CS_LOW()   			(GPIOA->BSRR = GPIO_BSRR_BR15)
#define MAX7219_CS_HIGH()  			(GPIOA->BSRR = GPIO_BSRR_BS15)

#define MCP2515_NODE2_CS_LOW()   	(GPIOA->BSRR = GPIO_BSRR_BR4)
#define MCP2515_NODE2_CS_HIGH()  	(GPIOA->BSRR = GPIO_BSRR_BS4)
