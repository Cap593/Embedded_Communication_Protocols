/*
 * i2c.h
 *
 *  Created on: 06-Jul-2026
 *      Author: HP
 */



#ifndef INC_I2C_H_
#define INC_I2C_H_

/*
 * I2C timeout for polling loops
 */

#define I2C_TIMEOUT                 100000U



void I2C1_GPIO_Init(void);
void I2C1_Init_100kHz(void);
uint8_t I2C1_MasterWrite(uint8_t slave_addr_7bit,
                         const uint8_t *tx_data,
                         uint16_t len);
uint8_t I2C1_MasterWriteRead(uint8_t slave_addr_7bit,
                             const uint8_t *tx_data,
                             uint16_t tx_len,
                             uint8_t *rx_data,
                             uint16_t rx_len);


#endif /* INC_I2C_H_ */
