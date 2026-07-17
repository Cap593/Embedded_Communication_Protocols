/*
 * can.h
 *
 *  Created on: 20-Jun-2026
 *      Author: HP
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "stm32f407xx.h"

#define CAN1_RESET_MASK		(1UL << 15UL)

void CAN1_GPIO_Init(void);
void CAN1_Init(void);
uint8_t CAN1_InitializationMode(void);
void CAN1_Reset(void);


#endif /* INC_CAN_H_ */
