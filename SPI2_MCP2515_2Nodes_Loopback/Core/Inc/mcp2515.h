/*
 * mcp2515.h
 *
 *  Created on: 10-Jun-2026
 *      Author: HP
 */

#ifndef INC_MCP2515_H_
#define INC_MCP2515_H_

#include "stm32f4xx_hal.h"

//MCP commands
#define MCP2515_CMD_RESET        0xC0
#define MCP2515_CMD_READ         0x03
#define MCP2515_CMD_WRITE        0x02
#define MCP2515_CMD_BIT_MODIFY   0x05
#define MCP2515_CMD_READ_STATUS  0xA0

//CAN modes
#define MCP2515_MODE_NORMAL      0x00
#define MCP2515_MODE_SLEEP       0x20
#define MCP2515_MODE_LOOPBACK    0x40
#define MCP2515_MODE_LISTEN_ONLY 0x60
#define MCP2515_MODE_CONFIG      0x80

//MCP registers
#define MCP2515_CANSTAT   0x0E
#define MCP2515_CANCTRL   0x0F

#define MCP2515_CNF3   0x28
#define MCP2515_CNF2   0x29
#define MCP2515_CNF1   0x2A

#define MCP2515_CANINTE     0x2B
#define MCP2515_CANINTF     0x2C

#define MCP2515_TXB0CTRL    0x30
#define MCP2515_TXB0SIDH    0x31
#define MCP2515_TXB0SIDL    0x32
#define MCP2515_TXB0DLC     0x35
#define MCP2515_TXB0D0      0x36

#define MCP2515_RXB0CTRL    0x60
#define MCP2515_RXB0SIDH    0x61
#define MCP2515_RXB0SIDL    0x62
#define MCP2515_RXB0DLC     0x65
#define MCP2515_RXB0D0      0x66

#define MCP2515_RXB1CTRL	0x70

//Masks
#define MCP2515_MODE_MASK   			0xE0
#define TRANSMIT_REQ_MASK   			0x08
#define MCP2515_CANINTF_RX0IF_MASK   	0x01
#define MCP2515_CANINTF_RX1IF_MASK   	0x02

void MCP2515_Init(void);
void MCP2515_Reset(void);
uint8_t MCP2515_ReadRegister(uint8_t reg);
uint8_t MCP2515_ReadStatus(void);
void MCP2515_WriteRegister(uint8_t address, uint8_t data);
void MCP2515_BitModify(uint8_t address,uint8_t mask,uint8_t data);
uint8_t MCP2515_GetMode(void);
void MCP2515_SetMode(uint8_t mode);
uint8_t MCP2515_SetModeVerify(uint8_t mode);


uint8_t MCP2515_SendMessage(uint16_t id,
                            uint8_t *data,
                            uint8_t len);

uint8_t MCP2515_ReadMessage(uint16_t *id,
                            uint8_t *data,
                            uint8_t *len);

void SPI1_Test(void);


#endif /* INC_MCP2515_H_ */
