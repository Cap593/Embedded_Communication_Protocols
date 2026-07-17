/*
 * at24c256.h
 *
 *  Created on: 07-Jul-2026
 *      Author: HP
 */

#ifndef INC_AT24C256_H_
#define INC_AT24C256_H_

#include "stm32f4xx_hal.h"

/*
 * AT24C256 default 7-bit I2C address.
 *
 * Device address format:
 * 1 0 1 0 0 A1 A0 R/W
 *
 * If A1 = 0 and A0 = 0:
 * 7-bit address = 0b1010000 = 0x50
 */
#define AT24C256_I2C_ADDR_7BIT    	0x50U
#define AT24C256_ADDR_WRITE   		(AT24C256_I2C_ADDR_7BIT << 1)
#define AT24C256_ADDR_READ    		((AT24C256_I2C_ADDR_7BIT << 1) | 1U)

/*
 * AT24C256 memory details
 */
#define AT24C256_SIZE_BYTES        32768U
#define AT24C256_LAST_ADDR         0x7FFFU
#define AT24C256_PAGE_SIZE         64U

#define PAGE_OFFSET_EEPROM(addr)    ((addr) & 0x3FUL)
#define PAGE_BASE_EEPROM(addr)      ((addr) & ~0x3FUL)
#define PAGE_SPACE_EEPROM(addr)	 (0x3FUL - PAGE_OFFSET)


uint8_t AT24C256_WriteByte(uint16_t mem_addr, uint8_t data);
uint8_t AT24C256_ReadByte(uint16_t mem_addr, uint8_t *read_data);
uint8_t AT24C256_WritePage(uint16_t mem_addr,
                           const uint8_t *data,
                           uint16_t len);
uint8_t AT24C256_ReadBuffer(uint16_t mem_addr,
                            uint8_t *data,
                            uint16_t len);
uint8_t AT24C256_WriteBuffer(uint16_t mem_addr,
                             const uint8_t *data,
                             uint16_t len);
uint8_t AT24C256_Fill(uint8_t value);
void AT24C256_TestAllApis(void);


#endif /* INC_AT24C256_H_ */
