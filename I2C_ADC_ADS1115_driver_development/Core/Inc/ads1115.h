/*
 * ads1115.h
 *
 *  Created on: 09-Jul-2026
 *      Author: HP
 */

#ifndef INC_ADS1115_H_
#define INC_ADS1115_H_

#include "stm32f4xx_hal.h"

#define ADS1115_ADDR_GND              0x48U

#define ADS1115_REG_CONVERSION        0x00U
#define ADS1115_REG_CONFIG            0x01U
#define ADS1115_REG_LO_THRESH         0x02U
#define ADS1115_REG_HI_THRESH         0x03U

/*
 * Config register default after reset = 0x8583
 */
#define ADS1115_CONFIG_RESET_VALUE     			   0x8583U
#define ADS1115_CONFIG_AIN0_SINGLE_4V096_128SPS    0xC383U
#define ADS1115_CONFIG_AIN0_CONT_4V096_128SPS      0xC283U
/*
 * For PGA = ±4.096 V:
 * LSB = 125 µV.
 */
#define ADS1115_LSB_4V096_UV                       125

uint8_t ADS1115_Init(void);

uint8_t ADS1115_WriteRegister(uint8_t dev_addr,
                              uint8_t reg_addr,
                              uint16_t reg_data);

uint8_t ADS1115_ReadRegister(uint8_t dev_addr,
                             uint8_t reg_addr,
                             uint16_t *reg_data);
void ADS1115_TestBasic(void);
uint8_t ADS1115_ReadRaw(int16_t *raw);
uint8_t ADS1115_StartSingleShot_AIN0(void);
uint8_t ADS1115_IsConversionReady(uint8_t *ready);
float ADS1115_RawToVoltage(int16_t raw);
void ADS1115_Test_AIN0(void);
uint8_t ADS1115_StartContinuous_AIN0(void);
uint8_t ADS1115_StopContinuous(void);
uint8_t ADS1115_ReadContinuousRaw(int16_t *raw);
uint8_t ADS1115_RawToMilliVolt_4V096(int16_t raw,
                                     uint16_t *voltage_mV);
uint8_t ADS1115_ReadContinuous_AIN0(int16_t *raw,
                                    uint16_t *voltage_mV);
void ADS1115_TestContinuous_AIN0(void);




#endif /* INC_ADS1115_H_ */
