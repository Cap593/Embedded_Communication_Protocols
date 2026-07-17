/*
 * mcp2515.c
 *
 *  Created on: 10-Jun-2026
 *      Author: HP
 */

#include "mcp2515.h"
#include "spi.h"
#include "usart.h"

static void MCP2515_CS_Low(void);
static void MCP2515_CS_High(void);
static uint8_t MCP2515_SPI_TransferByte(uint8_t data);
static uint8_t MCP2515_GetFreeTxBuffer(uint8_t *ctrl_addr,uint8_t *sidh_addr,
								uint8_t *sidl_addr,uint8_t *dlc_addr,uint8_t *d0_addr);

MCP2515_Node_t active_mcp_node = MCP2515_NODE_1;
MCP2515_SpiBus_t active_mcp_spi_bus = MCP2515_SPI_BUS_2;
extern uint8_t current_txctrl_addr;

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

static uint8_t MCP2515_GetFreeTxBuffer(uint8_t *ctrl_addr,uint8_t *sidh_addr,
								uint8_t *sidl_addr,uint8_t *dlc_addr,uint8_t *d0_addr)
{
	uint8_t buffer_available = E_NOT_OK;

	if((MCP2515_ReadRegister(MCP2515_TXB0CTRL)& 0x08) == 0)
	{
		//if buffer available then store register address
		buffer_available = E_OK;
		*ctrl_addr = MCP2515_TXB0CTRL;
		*sidh_addr = MCP2515_TXB0SIDH;
		*sidl_addr = MCP2515_TXB0SIDL;
		*dlc_addr  = MCP2515_TXB0DLC;
		*d0_addr   = MCP2515_TXB0D0;
	}
	else if((MCP2515_ReadRegister(MCP2515_TXB1CTRL)& 0x08) == 0)
	{
		//if buffer available then store register address
		buffer_available = E_OK;
		*ctrl_addr = MCP2515_TXB1CTRL;
		*sidh_addr = MCP2515_TXB1SIDH;
		*sidl_addr = MCP2515_TXB1SIDL;
		*dlc_addr  = MCP2515_TXB1DLC;
		*d0_addr   = MCP2515_TXB1D0;
	}
	else if((MCP2515_ReadRegister(MCP2515_TXB2CTRL)& 0x08) == 0)
	{
		//if buffer available then store register address
		buffer_available = E_OK;
		*ctrl_addr = MCP2515_TXB2CTRL;
		*sidh_addr = MCP2515_TXB2SIDH;
		*sidl_addr = MCP2515_TXB2SIDL;
		*dlc_addr  = MCP2515_TXB2DLC;
		*d0_addr   = MCP2515_TXB2D0;
	}
	else
	{
		buffer_available = E_NOT_OK;
	}

	return buffer_available;
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

	/* Enable Receive interrupt for buffer 0 and buffer 1 */
	MCP2515_BitModify(MCP2515_CANINTE, 0x03, 0x03);
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
    uint8_t ctrl_addr;
    uint8_t sidh_addr;
    uint8_t sidl_addr;
    uint8_t dlc_addr;
    uint8_t d0_addr;

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

    //find free buffer
    if(!MCP2515_GetFreeTxBuffer(&ctrl_addr,&sidh_addr,&sidl_addr,&dlc_addr,&d0_addr))
    {
    	return 0;
    }

    //global var for current tx addr
    current_txctrl_addr = ctrl_addr;
    /*
     * Standard ID mapping:
     * TXB0SIDH = SID10..SID3
     * TXB0SIDL[7:5] = SID2..SID0
     */
    sidh = (uint8_t)(id >> 3);
    sidl = (uint8_t)((id & 0x07) << 5);

    /* Load standard ID */
    MCP2515_WriteRegister(sidh_addr, sidh);
    MCP2515_WriteRegister(sidl_addr, sidl);

    /* Standard data frame, DLC = len */
    MCP2515_WriteRegister(dlc_addr, len & 0x0F);

    /* Load data bytes */
    for(uint8_t i = 0; i < len; i++)
    {
        MCP2515_WriteRegister(d0_addr + i, data[i]);
    }

    /* Request transmission: set TXB0CTRL.TXREQ */
    MCP2515_BitModify(ctrl_addr, 0x08, 0x08);

    return 1;
}

uint8_t MCP2515_ReadMessage(uint16_t *id, uint8_t *data, uint8_t *len)
{
    uint8_t sidh_addr;
    uint8_t sidl_addr;
    uint8_t dlc_addr;
    uint8_t d0_addr;
    uint8_t clear_mask;
    uint8_t canintf;

    uint8_t sidh;
    uint8_t sidl;
    uint8_t dlc;

    canintf = MCP2515_ReadRegister(MCP2515_CANINTF);

    if(canintf & MCP2515_CANINTF_RX0IF_MASK)
    {
        sidh_addr  = MCP2515_RXB0SIDH;
        sidl_addr  = MCP2515_RXB0SIDL;
        dlc_addr   = MCP2515_RXB0DLC;
        d0_addr    = MCP2515_RXB0D0;
        clear_mask = MCP2515_CANINTF_RX0IF_MASK;
    }
    else if(canintf & MCP2515_CANINTF_RX1IF_MASK)
    {
        sidh_addr  = MCP2515_RXB1SIDH;
        sidl_addr  = MCP2515_RXB1SIDL;
        dlc_addr   = MCP2515_RXB1DLC;
        d0_addr    = MCP2515_RXB1D0;
        clear_mask = MCP2515_CANINTF_RX1IF_MASK;
    }
    else
    {
        return 0;
    }

    sidh = MCP2515_ReadRegister(sidh_addr);
    sidl = MCP2515_ReadRegister(sidl_addr);

    *id = ((uint16_t)sidh << 3) | (sidl >> 5);

    dlc = MCP2515_ReadRegister(dlc_addr) & 0x0F;

    if(dlc > 8)
    {
        dlc = 8;
    }

    *len = dlc;

    for(uint8_t i = 0; i < dlc; i++)
    {
        data[i] = MCP2515_ReadRegister(d0_addr + i);
    }

    MCP2515_BitModify(MCP2515_CANINTF, clear_mask, 0x00);

    return 1;
}

uint8_t MCP2515_PrintTxStatus(uint8_t txctrl_addr)
{
    uint8_t TxErrorFlag = 1;
    uint8_t txctrl = MCP2515_ReadRegister(txctrl_addr);

    if(txctrl & 0x08)
    {
        USART2_SendString("TX still pending\r\n");
        TxErrorFlag = 0;
    }

    if(txctrl & 0x10)
    {
        USART2_SendString("TX transmit error\r\n");
        TxErrorFlag = 0;
    }

    if(txctrl & 0x20)
    {
        USART2_SendString("TX lost arbitration\r\n");
        TxErrorFlag = 0;
    }

    if(txctrl & 0x40)
    {
        USART2_SendString("TX aborted\r\n");
        TxErrorFlag = 0;
    }

    return TxErrorFlag;
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

void MCP2515_INT_PA0_Init(void)
{
    /*
     * MCP2515 INT pin:
     * Active low
     * Normal state = high
     * Interrupt event = falling edge
     */

    /* Enable GPIOA clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* Enable SYSCFG clock */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /*
     * PA0 as input
     * MODER0 = 00
     */
    GPIOA->MODER &= ~GPIO_MODER_MODER0;

    /*
     * Pull-up on PA0
     * PUPDR0 = 01
     */
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;
    GPIOA->PUPDR |=  GPIO_PUPDR_PUPD0_0;

    /*
     * Map EXTI0 to Port A.
     * EXTICR[0], EXTI0 field = bits [3:0]
     * Port A = 0000
     */
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;

    /*
     * Unmask EXTI0 interrupt
     */
    EXTI->IMR |= EXTI_IMR_MR0;

    /*
     * Falling edge trigger enable.
     * MCP2515 INT is active-low.
     */
    EXTI->FTSR |= EXTI_FTSR_TR0;

    /*
     * Disable rising edge trigger
     */
    EXTI->RTSR &= ~EXTI_RTSR_TR0;

    /*
     * Clear pending flag before enabling NVIC.
     * Write 1 to clear.
     */
    EXTI->PR = EXTI_PR_PR0;

    /*
     * Optional but recommended: set priority
     */
    NVIC_SetPriority(EXTI0_IRQn, 2);

    /*
     * Enable EXTI line 0 interrupt in NVIC.
     */
    NVIC_EnableIRQ(EXTI0_IRQn);
}

void MCP2515_INT_PA1_Init(void)
{
    /*
     * MCP2515 INT pin:
     * Active low
     * Normal state = high
     * Interrupt event = falling edge
     */

    /* Enable GPIOA clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* Enable SYSCFG clock */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /*
     * PA1 as input
     * MODER0 = 00
     */
    GPIOA->MODER &= ~GPIO_MODER_MODER1;

    /*
     * Pull-up on PA1
     * PUPDR0 = 01
     */
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD1;
    GPIOA->PUPDR |=  GPIO_PUPDR_PUPD1_0;

    /*
     * Map EXTI0 to Port A.
     * EXTICR[0], EXTI0 field = bits [3:0]
     * Port A = 0000
     */
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI1;

    /*
     * Unmask EXTI0 interrupt
     */
    EXTI->IMR |= EXTI_IMR_MR1;

    /*
     * Falling edge trigger enable.
     * MCP2515 INT is active-low.
     */
    EXTI->FTSR |= EXTI_FTSR_TR1;

    /*
     * Disable rising edge trigger
     */
    EXTI->RTSR &= ~EXTI_RTSR_TR1;

    /*
     * Clear pending flag before enabling NVIC.
     * Write 1 to clear.
     */
    EXTI->PR = EXTI_PR_PR1;

    /*
     * Optional but recommended: set priority
     */
    NVIC_SetPriority(EXTI1_IRQn, 2);

    /*
     * Enable EXTI line 0 interrupt in NVIC.
     */
    NVIC_EnableIRQ(EXTI1_IRQn);
}

uint8_t MCP2515_GetInterruptFlags(void)
{
    return MCP2515_ReadRegister(MCP2515_CANINTF);
}


