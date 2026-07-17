/*
 * mcp2515.c
 *
 *  Created on: 10-Jun-2026
 *      Author: HP
 */

#include "mcp2515.h"
#include "spi.h"

void MCP2515_Reset(void)
{
	MCP2515_CS_LOW();
	SPI2_TransferByte(MCP2515_CMD_RESET);
	MCP2515_CS_HIGH();

	// Needed for oscillator stabilization
	HAL_Delay(120);
}

uint8_t MCP2515_ReadRegister(uint8_t reg)
{
	uint8_t reg_data;

	MCP2515_CS_LOW();
	SPI2_TransferByte(MCP2515_CMD_READ);
	SPI2_TransferByte(reg);
	reg_data = SPI2_TransferByte(0xFF);
	MCP2515_CS_HIGH();

	return reg_data;
}

uint8_t MCP2515_ReadStatus(void)
{
	uint8_t reg_data;

	MCP2515_CS_LOW();
	SPI2_TransferByte(MCP2515_CMD_READ_STATUS);
	reg_data = SPI2_TransferByte(0xFF);
	MCP2515_CS_HIGH();

	return reg_data;
}

void MCP2515_WriteRegister(uint8_t address, uint8_t data)
{
	MCP2515_CS_LOW();
	SPI2_TransferByte(MCP2515_CMD_WRITE);
	SPI2_TransferByte(address);
	SPI2_TransferByte(data);
	MCP2515_CS_HIGH();
}

void MCP2515_BitModify(uint8_t address,uint8_t mask,uint8_t data)
{
	MCP2515_CS_LOW();
	SPI2_TransferByte(MCP2515_CMD_BIT_MODIFY);
	SPI2_TransferByte(address);
	SPI2_TransferByte(mask);
	SPI2_TransferByte(data);
	MCP2515_CS_HIGH();
}

uint8_t MCP2515_GetMode(void)
{
    uint8_t canstat;

    canstat = MCP2515_ReadRegister(MCP2515_CANSTAT);

    return (canstat & MCP2515_MODE_MASK);
}

void MCP2515_SetMode(uint8_t mode)
{
    MCP2515_BitModify(MCP2515_CANCTRL,
                      MCP2515_MODE_MASK,
                      mode);
}

uint8_t MCP2515_SetModeVerify(uint8_t mode)
{
    uint32_t timeout = 100000;

    MCP2515_SetMode(mode);

    while(timeout--)
    {
        if(MCP2515_GetMode() == mode)
        {
            return 1;   // success
        }
    }

    return 0;           // failed
}



