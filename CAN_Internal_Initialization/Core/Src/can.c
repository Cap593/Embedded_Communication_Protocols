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
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /*
     * PB8  = CAN1_RX
     * PB9  = CAN1_TX
     * AF9  = CAN1
     */

    /* Clear mode bits */
    GPIOB->MODER &= ~((3UL << (8 * 2)) |
                      (3UL << (9 * 2)));

    /* Alternate function mode */
    GPIOB->MODER |=  ((2UL << (8 * 2)) |
                      (2UL << (9 * 2)));

    /* Push-pull */
    GPIOB->OTYPER &= ~((1UL << 8) |
                       (1UL << 9));

    /* High speed */
    GPIOB->OSPEEDR &= ~((3UL << (8 * 2)) |
                        (3UL << (9 * 2)));

    GPIOB->OSPEEDR |=  ((2UL << (8 * 2)) |
                        (2UL << (9 * 2)));

    /*
     * CAN RX recessive is high.
     * Pull-up on RX is useful.
     */
    GPIOB->PUPDR &= ~((3UL << (8 * 2)) |
                      (3UL << (9 * 2)));

    GPIOB->PUPDR |= (1UL << (8 * 2));   /* PB8 pull-up */

    /* AF9 for PB8 and PB9 */
    GPIOB->AFR[1] &= ~((0xFUL << ((8 - 8) * 4)) |
                       (0xFUL << ((9 - 8) * 4)));

    GPIOB->AFR[1] |=  ((9UL << ((8 - 8) * 4)) |
                       (9UL << ((9 - 8) * 4)));
}

void CAN1_Init(void)
{
	//Enable clock for the CAN peripheral
	RCC->APB1ENR |= (1 << 25);

	if(CAN1_InitializationMode())
	{
		USART2_SendString("Internal CAN1 Initialization Mode OK!!!\r\n");
	}

}

void CAN1_Reset(void)
{
	//force CAN1 reset , will result in sleep mode
	CAN1->MCR |= CAN1_RESET_MASK;
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


