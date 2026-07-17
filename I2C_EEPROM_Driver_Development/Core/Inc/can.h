/*
 * can.h
 *
 *  Created on: 20-Jun-2026
 *      Author: HP
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "stm32f407xx.h"
#include "common.h"

#define CAN1_RESET_MASK		(1UL << 15UL)

void CAN1_GPIO_Init(void);
uint8_t CAN1_Init(void);
void CAN1_Filter_AcceptAll_FIFO0(void);
uint8_t CAN1_InitializationMode(void);
void CAN1_Reset(void);
uint8_t CAN1_SendStdMessage(uint16_t id,
							uint8_t *data,
							uint8_t len);
uint8_t CAN1_ReadMessage_FIFO0(uint16_t *id,
							uint8_t *data,
							uint8_t *len);
uint8_t CAN1_Init_Loopback_500kbps(void);
uint8_t CAN1_Init_Normal_500kbps(void);
void CAN1_RX0_Interrupt_Enable(void);

#endif /* INC_CAN_H_ */
