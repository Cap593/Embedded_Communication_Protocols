/*
 * can.c
 *
 *  Created on: 20-Jun-2026
 *      Author: HP
 */

#include "can.h"
#include "usart.h"

void CAN1_GPIO_Init(void)
{
    /* Enable GPIOB clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    /*
     * PD0  = CAN1_RX
     * PD1  = CAN1_TX
     * AF9  = CAN1
     */

    /* Clear mode bits */
    GPIOD->MODER &= ~((3UL << (0 * 2)) |
                      (3UL << (1 * 2)));

    /* Alternate function mode */
    GPIOD->MODER |=  ((2UL << (0 * 2)) |
                      (2UL << (1 * 2)));

    /* Push-pull */
    GPIOD->OTYPER &= ~((1UL << 0) |
                       (1UL << 1));

    /* High speed */
    GPIOD->OSPEEDR &= ~((3UL << (0 * 2)) |
                        (3UL << (1 * 2)));

    GPIOD->OSPEEDR |=  ((2UL << (0 * 2)) |
                        (2UL << (1 * 2)));

    /*
     * CAN RX recessive is high.
     * Pull-up on RX is useful.
     */
    GPIOD->PUPDR &= ~((3UL << (0 * 2)) |
                      (3UL << (1 * 2)));

    GPIOD->PUPDR |= (1UL << (0 * 2));   /* PB8 pull-up */

    /* AF9 for PB8 and PB9 */
    GPIOD->AFR[0] &= ~((GPIO_AFRL_AFSEL0) |
                       (GPIO_AFRL_AFSEL1));

    GPIOD->AFR[0] |=  (9UL << (GPIO_AFRL_AFSEL0_Pos)) |
    				  (9UL << (GPIO_AFRL_AFSEL1_Pos));
}

uint8_t CAN1_Init_Loopback_500kbps(void)
{
	uint32_t timeout = 0;

	//Enable clock for the CAN peripheral
	RCC->APB1ENR |= (1 << 25);

	if(CAN1_InitializationMode())
	{
		USART2_SendString("Internal CAN1 Initialization Mode OK!!!\r\n");
	}

	/*
	     * MCR configuration
	     *
	     * TTCM = 0: time-triggered mode disabled
	     * ABOM = 0: automatic bus-off management disabled
	     * AWUM = 0: automatic wakeup disabled
	     * NART = 0: automatic retransmission enabled
	     * RFLM = 0: FIFO not locked
	     * TXFP = 0: transmit priority by identifier
	     */
	    CAN1->MCR &= ~(CAN_MCR_TTCM |
	                   CAN_MCR_ABOM |
	                   CAN_MCR_AWUM |
	                   CAN_MCR_NART |
	                   CAN_MCR_RFLM |
	                   CAN_MCR_TXFP);

	    /*
	     * Bit timing for PCLK1 = 32 MHz, 500 kbps
	     *
	     * Baudrate = PCLK1 / [Prescaler × (1 + BS1 + BS2)]
	     *
	     * Prescaler = 8
	     * BS1       = 5
	     * BS2       = 2
	     * SJW       = 1
	     *
	     * Total TQ = 1 + 5 + 2 = 8
	     * Baudrate = 32 MHz / (8 × 8) = 500 kbps
	     *
	     * Register encoding:
	     * BRP = Prescaler - 1 = 7
	     * TS1 = BS1 - 1 = 4
	     * TS2 = BS2 - 1 = 1
	     * SJW = SJW - 1 = 0
	     */
	    CAN1->BTR = 0;

	    CAN1->BTR |= (7UL << CAN_BTR_BRP_Pos);   /* Prescaler - 1 */
	    CAN1->BTR |= (4UL << CAN_BTR_TS1_Pos);   /* BS1 - 1 */
	    CAN1->BTR |= (1UL << CAN_BTR_TS2_Pos);   /* BS2 - 1 */
	    CAN1->BTR |= (0UL << CAN_BTR_SJW_Pos);   /* SJW - 1 */

	    /*
	     * Loopback mode enabled.
	     * Silent mode disabled.
	     */
	    CAN1->BTR |= CAN_BTR_LBKM;
	    CAN1->BTR &= ~CAN_BTR_SILM;

	    /*
		 * Configure filters before leaving init mode
		 */
		CAN1_Filter_AcceptAll_FIFO0();

		/*
		 * Leave initialization mode
		 */
		CAN1->MCR &= ~CAN_MCR_INRQ;

		timeout = 1000000;
		while((CAN1->MSR & CAN_MSR_INAK) && timeout)
		{
			timeout--;
		}

		if(timeout == 0)
		{
			return 0;
		}

		return 1;

}

uint8_t CAN1_Init_Normal_500kbps(void)
{
    uint32_t timeout;

    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;

    /* Leave sleep mode */
    CAN1->MCR &= ~CAN_MCR_SLEEP;

    /* Request initialization mode */
    CAN1->MCR |= CAN_MCR_INRQ;

    timeout = 1000000;
    while(((CAN1->MSR & CAN_MSR_INAK) == 0) && timeout)
    {
        timeout--;
    }

    if(timeout == 0)
    {
        return 0;
    }

    CAN1->MCR &= ~(CAN_MCR_TTCM |
                   CAN_MCR_ABOM |
                   CAN_MCR_AWUM |
                   CAN_MCR_NART |
                   CAN_MCR_RFLM |
                   CAN_MCR_TXFP);

    /*
     * Same 500 kbps timing as loopback:
     * PCLK1 = 32 MHz
     * Prescaler = 8
     * BS1 = 5
     * BS2 = 2
     * Total TQ = 8
     */
    CAN1->BTR = 0;
    CAN1->BTR |= (7UL << CAN_BTR_BRP_Pos);
    CAN1->BTR |= (4UL << CAN_BTR_TS1_Pos);
    CAN1->BTR |= (1UL << CAN_BTR_TS2_Pos);
    CAN1->BTR |= (0UL << CAN_BTR_SJW_Pos);

    /*
     * Normal mode:
     * loopback disabled
     * silent disabled
     */
    CAN1->BTR &= ~CAN_BTR_LBKM;
    CAN1->BTR &= ~CAN_BTR_SILM;

    /* In case of Normal mode , no need of RX pull up .
     * Recessive bits will be received from the bus
     */
    GPIOD->OTYPER &= ~(3UL << (0 * 2));

    /* Initialize filter config */
    CAN1_Filter_AcceptAll_FIFO0();

    /* Leave initialization mode */
    CAN1->MCR &= ~CAN_MCR_INRQ;

    timeout = 1000000;
    while((CAN1->MSR & CAN_MSR_INAK) && timeout)
    {
        timeout--;
    }

    if(timeout == 0)
    {
        return 0;
    }


    return 1;
}

void CAN1_Reset(void)
{
	//force CAN1 reset , will result in sleep mode
	CAN1->MCR |= CAN1_RESET_MASK;
}

void CAN1_RX0_Interrupt_Enable(void)
{
    CAN1->IER |= CAN_IER_FMPIE0;

    NVIC_SetPriority(CAN1_RX0_IRQn, 1);
    NVIC_EnableIRQ(CAN1_RX0_IRQn);
}

uint8_t CAN1_InitializationMode(void)
{
	uint32_t timeout;

	//leave sleep mode
	CAN1->MCR &= ~(1 << 1);

	//initialization Request
	CAN1->MCR |= (1 << 0);

	timeout = 1000000;
	while(((CAN1->MSR & CAN_MSR_INAK) == 0) && timeout)
	{
		timeout--;
	}

	if(timeout == 0)
	{
		return 0;
	}

	return 1;

}

void CAN1_Filter_AcceptAll_FIFO0(void)
{
    /*
     * Enter filter initialization mode.
     * Reception is disabled while FINIT = 1.
     */
    CAN1->FMR |= CAN_FMR_FINIT;

    /* Deactivate filter bank 0 */
    CAN1->FA1R &= ~(1UL << 0);

    /*
     * Identifier mask mode:
     * 0 = mask mode
     */
    CAN1->FM1R &= ~(1UL << 0);

    /*
     * 32-bit scale:
     * 1 = one 32-bit filter
     */
    CAN1->FS1R |= (1UL << 0);

    /*
     * Assign filter bank 0 to FIFO0:
     * 0 = FIFO0
     */
    CAN1->FFA1R &= ~(1UL << 0);

    /*
     * Accept all:
     * ID = 0
     * Mask = 0
     */
    CAN1->sFilterRegister[0].FR1 = 0x00000000;
    CAN1->sFilterRegister[0].FR2 = 0x00000000;

    /* Activate filter bank 0 */
    CAN1->FA1R |= (1UL << 0);

    /* Leave filter initialization mode */
    CAN1->FMR &= ~CAN_FMR_FINIT;
}

uint8_t CAN1_SendStdMessage(uint16_t id, uint8_t *data, uint8_t len)
{
    uint8_t mailbox;
    uint32_t tdlr = 0;
    uint32_t tdhr = 0;

    if(id > 0x7FF)
    {
        return E_NOT_OK;
    }

    if(len > 8)
    {
        return E_NOT_OK;
    }

    if((data == 0) && (len > 0))
    {
        return E_NOT_OK;
    }

    /*
     * Find empty transmit mailbox.
     * TME0 = bit 26
     * TME1 = bit 27
     * TME2 = bit 28
     */
    if(CAN1->TSR & CAN_TSR_TME0)
    {
        mailbox = 0;
    }
    else if(CAN1->TSR & CAN_TSR_TME1)
    {
        mailbox = 1;
    }
    else if(CAN1->TSR & CAN_TSR_TME2)
    {
        mailbox = 2;
    }
    else
    {
        return E_NOT_OK;
    }

    /*
     * Prepare identifier register.
     *
     * Standard ID goes to STID[10:0] = bits [31:21]
     * IDE = 0 for standard ID
     * RTR = 0 for data frame
     * TXRQ = 0 initially
     */
    CAN1->sTxMailBox[mailbox].TIR = ((uint32_t)id << 21);

    /*
     * DLC in bits [3:0]
     */
    CAN1->sTxMailBox[mailbox].TDTR = (len & 0x0F);

    /*
     * Pack data bytes.
     */
    if(len > 0) tdlr |= ((uint32_t)data[0] << 0);
    if(len > 1) tdlr |= ((uint32_t)data[1] << 8);
    if(len > 2) tdlr |= ((uint32_t)data[2] << 16);
    if(len > 3) tdlr |= ((uint32_t)data[3] << 24);

    if(len > 4) tdhr |= ((uint32_t)data[4] << 0);
    if(len > 5) tdhr |= ((uint32_t)data[5] << 8);
    if(len > 6) tdhr |= ((uint32_t)data[6] << 16);
    if(len > 7) tdhr |= ((uint32_t)data[7] << 24);

    CAN1->sTxMailBox[mailbox].TDLR = tdlr;
    CAN1->sTxMailBox[mailbox].TDHR = tdhr;

    /*
     * Request transmission.
     */
    CAN1->sTxMailBox[mailbox].TIR |= CAN_TI0R_TXRQ;

    return E_OK;
}

uint8_t CAN1_ReadMessage_FIFO0(uint16_t *id, uint8_t *data, uint8_t *len)
{
    uint32_t rir;
    uint32_t rdtr;
    uint32_t rdlr;
    uint32_t rdhr;
    uint8_t dlc;

    /*
     * FMP0[1:0] = number of pending messages in FIFO0.
     */
    if((CAN1->RF0R & CAN_RF0R_FMP0) == 0)
    {
        return E_NOT_OK;
    }

    rir  = CAN1->sFIFOMailBox[0].RIR;
    rdtr = CAN1->sFIFOMailBox[0].RDTR;
    rdlr = CAN1->sFIFOMailBox[0].RDLR;
    rdhr = CAN1->sFIFOMailBox[0].RDHR;

    /*
     * Standard ID:
     * STID[10:0] is in RIR[31:21]
     */
    *id = (uint16_t)((rir >> 21) & 0x7FF);

    /*
     * DLC:
     * bits [3:0]
     */
    dlc = (uint8_t)(rdtr & 0x0F);

    if(dlc > 8)
    {
        dlc = 8;
    }

    *len = dlc;

    /*
     * Unpack data bytes.
     */
    if(dlc > 0) data[0] = (uint8_t)((rdlr >> 0)  & 0xFF);
    if(dlc > 1) data[1] = (uint8_t)((rdlr >> 8)  & 0xFF);
    if(dlc > 2) data[2] = (uint8_t)((rdlr >> 16) & 0xFF);
    if(dlc > 3) data[3] = (uint8_t)((rdlr >> 24) & 0xFF);

    if(dlc > 4) data[4] = (uint8_t)((rdhr >> 0)  & 0xFF);
    if(dlc > 5) data[5] = (uint8_t)((rdhr >> 8)  & 0xFF);
    if(dlc > 6) data[6] = (uint8_t)((rdhr >> 16) & 0xFF);
    if(dlc > 7) data[7] = (uint8_t)((rdhr >> 24) & 0xFF);

    /*
     * Release FIFO0 output mailbox.
     * This is like clearing RX flag in MCP2515.
     */
    CAN1->RF0R |= CAN_RF0R_RFOM0;

    return E_OK;
}


