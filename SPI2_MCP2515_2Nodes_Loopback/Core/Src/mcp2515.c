/*
 * mcp2515.c
 *
 *  Created on: 10-Jun-2026
 *      Author: HP
 */

#include "mcp2515.h"
#include "spi.h"

static void MCP2515_CS_Low(void);
static void MCP2515_CS_High(void);
static uint8_t MCP2515_SPI_TransferByte(uint8_t data);

MCP2515_Node_t active_mcp_node = MCP2515_NODE_1;
MCP2515_SpiBus_t active_mcp_spi_bus = MCP2515_SPI_BUS_2;

static uint8_t MCP2515_SPI_TransferByte(uint8_t data)
{
    if(active_mcp_spi_bus == MCP2515_SPI_BUS_1)
    {
        return SPI1_TransferByte(data);
    }
    else
    {
        return SPI2_TransferByte(data);
    }
}

static void MCP2515_CS_Low(void)
{
	if(active_mcp_node == MCP2515_NODE_1)
	{
		MCP2515_CS_LOW();
	}
	else
	{
		MCP2515_NODE2_CS_LOW();
	}
}

static void MCP2515_CS_High(void)
{
	if(active_mcp_node == MCP2515_NODE_1)
	{
		MCP2515_CS_HIGH();
	}
	else
	{
		MCP2515_NODE2_CS_HIGH();
	}
}

void MCP2515_SelectSpiBus(MCP2515_SpiBus_t bus)
{
    active_mcp_spi_bus = bus;
}

void MCP2515_SelectNode(MCP2515_Node_t node)
{
	active_mcp_node = node;
}

void MCP2515_Init(void)
{
	// Reset to reach config mode
	MCP2515_Reset();

	//check and wait till config mode
	while((MCP2515_ReadRegister(MCP2515_CANSTAT) & MCP2515_MODE_MASK) != MCP2515_MODE_CONFIG);

	/*Baudrate config , 500kbps ,8 N_TQ, SyncSeg = 1 TQ,
	PropSeg = 2 TQ ,PS1     = 3 TQ ,PS2     = 2 TQ
	Total   = 8 TQ*/
	MCP2515_WriteRegister(MCP2515_CNF1, 0x00);
	MCP2515_WriteRegister(MCP2515_CNF2, 0x91);
	MCP2515_WriteRegister(MCP2515_CNF3, 0x01);

	//Filter,mask settings, receive any message ID
	MCP2515_BitModify(MCP2515_RXB0CTRL, 0x60 , 0x60);
	MCP2515_BitModify(MCP2515_RXB1CTRL, 0x60 , 0x60);

	//enable RXB0 rollover to RXB1 */
	MCP2515_BitModify(MCP2515_RXB0CTRL, 0x04, 0x04);

	/* Clear interrupt flags */
	MCP2515_WriteRegister(MCP2515_CANINTF, 0x00);
}

void MCP2515_Reset(void)
{
	MCP2515_CS_Low();
	MCP2515_SPI_TransferByte(MCP2515_CMD_RESET);
	MCP2515_CS_High();

	// Needed for oscillator stabilization
	HAL_Delay(100);
}

uint8_t MCP2515_ReadRegister(uint8_t reg)
{
	uint8_t reg_data;

	MCP2515_CS_Low();
	MCP2515_SPI_TransferByte(MCP2515_CMD_READ);
	MCP2515_SPI_TransferByte(reg);
	reg_data = MCP2515_SPI_TransferByte(0xFF);
	MCP2515_CS_High();

	return reg_data;
}

uint8_t MCP2515_ReadStatus(void)
{
	uint8_t reg_data;

	MCP2515_CS_Low();
	MCP2515_SPI_TransferByte(MCP2515_CMD_READ_STATUS);
	reg_data = MCP2515_SPI_TransferByte(0xFF);
	MCP2515_CS_High();

	return reg_data;
}

void MCP2515_WriteRegister(uint8_t address, uint8_t data)
{
	MCP2515_CS_Low();
	MCP2515_SPI_TransferByte(MCP2515_CMD_WRITE);
	MCP2515_SPI_TransferByte(address);
	MCP2515_SPI_TransferByte(data);
	MCP2515_CS_High();
}

void MCP2515_BitModify(uint8_t address,uint8_t mask,uint8_t data)
{
	MCP2515_CS_Low();
	MCP2515_SPI_TransferByte(MCP2515_CMD_BIT_MODIFY);
	MCP2515_SPI_TransferByte(address);
	MCP2515_SPI_TransferByte(mask);
	MCP2515_SPI_TransferByte(data);
	MCP2515_CS_High();
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


uint8_t MCP2515_SendMessage(uint16_t id, uint8_t *data, uint8_t len)
{
    uint8_t sidh;
    uint8_t sidl;

    /* Standard CAN ID is 11-bit */
    if(id > 0x7FF)
    {
        return 0;
    }

    /* CAN data length max is 8 bytes */
    if(len > 8)
    {
        return 0;
    }

    if((data == 0) && (len > 0))
    {
        return 0;
    }

    /* Check TXB0 is free: TXREQ must be 0 */
    if(MCP2515_ReadRegister(MCP2515_TXB0CTRL) & 0x08)
    {
        return 0;
    }

    /*
     * Standard ID mapping:
     * TXB0SIDH = SID10..SID3
     * TXB0SIDL[7:5] = SID2..SID0
     */
    sidh = (uint8_t)(id >> 3);
    sidl = (uint8_t)((id & 0x07) << 5);

    /* Load standard ID */
    MCP2515_WriteRegister(MCP2515_TXB0SIDH, sidh);
    MCP2515_WriteRegister(MCP2515_TXB0SIDL, sidl);

    /* Standard data frame, DLC = len */
    MCP2515_WriteRegister(MCP2515_TXB0DLC, len & 0x0F);

    /* Load data bytes */
    for(uint8_t i = 0; i < len; i++)
    {
        MCP2515_WriteRegister(MCP2515_TXB0D0 + i, data[i]);
    }

    /* Request transmission: set TXB0CTRL.TXREQ */
    MCP2515_BitModify(MCP2515_TXB0CTRL, 0x08, 0x08);

    return 1;
}

uint8_t MCP2515_ReadMessage(uint16_t *id, uint8_t *data, uint8_t *len)
{
    uint8_t sidh;
    uint8_t sidl;
    uint8_t dlc;
    uint8_t canintf;

    /* Check if RXB0 has received message */
    canintf = MCP2515_ReadRegister(MCP2515_CANINTF);

    if((canintf & MCP2515_CANINTF_RX0IF_MASK) == 0)
    {
        return 0;   /* No message available */
    }

    /* Read standard identifier registers */
    sidh = MCP2515_ReadRegister(MCP2515_RXB0SIDH);
    sidl = MCP2515_ReadRegister(MCP2515_RXB0SIDL);

    /*
     * Reconstruct 11-bit standard ID:
     * SIDH contains SID10..SID3
     * SIDL[7:5] contains SID2..SID0
     */
    *id = ((uint16_t)sidh << 3) | (sidl >> 5);

    /* Read DLC */
    dlc = MCP2515_ReadRegister(MCP2515_RXB0DLC);
    dlc = dlc & 0x0F;

    if(dlc > 8)
    {
        dlc = 8;
    }

    *len = dlc;

    /* Read data bytes */
    for(uint8_t i = 0; i < dlc; i++)
    {
        data[i] = MCP2515_ReadRegister(MCP2515_RXB0D0 + i);
    }

    /*
     * Clear RX0IF after reading message.
     * This tells MCP2515 RXB0 is free again.
     */
    MCP2515_BitModify(MCP2515_CANINTF,
                      MCP2515_CANINTF_RX0IF_MASK,
                      0x00);

    return 1;
}

void SPI2_Test(void)
{
	uint8_t tx_data[] = {0xAA, 0x55, 0x96, 0x3C};

	MCP2515_SelectNode(MCP2515_NODE_1);
	MCP2515_CS_Low();

	for(uint8_t i=0; i<4 ;i++)
	{
		MCP2515_SPI_TransferByte(tx_data[i]);
	}

	MCP2515_CS_High();
}

void SPI1_Test(void)
{
	uint8_t tx_data[] = {0xAA, 0x55, 0x96, 0x3C};

	MCP2515_SelectNode(MCP2515_NODE_1);
	MCP2515_CS_Low();
	for(uint8_t i=0; i<4 ;i++)
	{
		MCP2515_SPI_TransferByte(tx_data[i]);
	}
	MCP2515_CS_High();
}


