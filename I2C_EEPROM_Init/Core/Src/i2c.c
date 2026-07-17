/*
 * i2c.c
 *
 *  Created on: 06-Jul-2026
 *      Author: HP
 */

#include "stm32f4xx_hal.h"
#include "i2c.h"
#include "common.h"

/*
 * Clear ADDR flag.
 *
 * In STM32F4 I2C, ADDR is cleared by:
 * 1. Read SR1
 * 2. Read SR2
 *
 * This sequence is mandatory.
 */
static void I2C1_ClearADDR(void)
{
    volatile uint32_t dummy;

    dummy = I2C1->SR1;
    dummy = I2C1->SR2;

    (void)dummy;
}



void AT24C256_I2C1_GPIO_Init(void)
{
    /*
     * PB6 = I2C1_SCL
     * PB7 = I2C1_SDA
     * AF4 = I2C1
     *
     * I2C pins must be:
     * - Alternate function
     * - Open-drain
     * - Pull-up enabled or external pull-up available
     */

    /* Enable GPIOB clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /*
     * Clear mode bits for PB6 and PB7
     */
    GPIOB->MODER &= ~((3UL << (6U * 2U)) |
                      (3UL << (7U * 2U)));

    /*
     * Set PB6 and PB7 to Alternate Function mode
     * MODER = 10
     */
    GPIOB->MODER |=  ((2UL << (6U * 2U)) |
                      (2UL << (7U * 2U)));

    /*
     * Configure PB6 and PB7 as open-drain.
     *
     * I2C bus uses open-drain because multiple devices
     * share the same SDA/SCL lines.
     */
    GPIOB->OTYPER |= ((1UL << 6U) |
                      (1UL << 7U));

    /*
     * Configure speed.
     * Medium/high is fine for 100 kHz / 400 kHz I2C.
     */
    GPIOB->OSPEEDR &= ~((3UL << (6U * 2U)) |
                        (3UL << (7U * 2U)));

    GPIOB->OSPEEDR |=  ((2UL << (6U * 2U)) |
                        (2UL << (7U * 2U)));

    /*
     * Enable internal pull-up.
     *
     * External pull-ups are preferred.
     * Your EEPROM module may already have 10k pull-ups.
     * Internal pull-up is still okay for first testing.
     */
    GPIOB->PUPDR &= ~((3UL << (6U * 2U)) |
                      (3UL << (7U * 2U)));

    GPIOB->PUPDR |=  ((1UL << (6U * 2U)) |
                      (1UL << (7U * 2U)));

    /*
     * Select AF4 for PB6 and PB7.
     *
     * AFR[0] handles pins 0 to 7.
     */
    GPIOB->AFR[0] &= ~((0xFUL << (6U * 4U)) |
                       (0xFUL << (7U * 4U)));

    GPIOB->AFR[0] |=  ((4UL << (6U * 4U)) |
                       (4UL << (7U * 4U)));
}

void AT24C256_I2C1_Init(void)
{
    /*
     * Enable I2C1 peripheral clock.
     * I2C1 is on APB1.
     */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    /*
     * Disable I2C before configuration.
     */
    I2C1->CR1 &= ~I2C_CR1_PE;


    /*
     * Software reset.
     * Useful during development to recover from unknown I2C state.
     */
    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    /*
     * Use normal I2C mode, not SMBus.
     */
    I2C1->CR1 &= ~I2C_CR1_SMBUS;

    /*
     * CR2.FREQ[5:0]
     *
     * This must be programmed with APB1 peripheral clock in MHz.
     * Your PCLK1 = 32 MHz.
     *
     * Important:
     * This does NOT mean I2C speed is 32 MHz.
     * It tells the I2C peripheral its input clock.
     */
    I2C1->CR2 |= (32U << I2C_CR2_FREQ_Pos);

    /*
     * Configure standard mode: 100 kHz.
     *
     * In standard mode:
     * SCL high time = CCR * TPCLK1
     * SCL low time  = CCR * TPCLK1
     *
     * CCR = PCLK1 / (2 * I2C_speed)
     * CCR = 32 MHz / (2 * 100 kHz)
     * CCR = 160
     */
    I2C1->CCR &= ~(1 << 15);
    I2C1->CCR |= (160U << 0);

    /*
     * Configure maximum rise time.
     *
     * For standard mode:
     * TRISE = FREQ + 1
     * TRISE = 32 + 1 = 33
     */
    I2C1->TRISE |= (33U << 0);


    /*
     * Enable ACK by default.
     * During read operations, we will disable ACK for last byte.
     */
    I2C1->CR1 |= I2C_CR1_ACK;

    /*
     * Enable I2C peripheral.
     */
    I2C1->CR1 |= I2C_CR1_PE;

}

/*
 * START
 * 0xA0 ACK
 * memory address high byte ACK
 * memory address low byte ACK
 * data byte ACK
 * STOP
*/

uint8_t AT24C256_WriteByte(uint16_t mem_addr, uint8_t data)
{
	uint8_t write_stat = E_NOT_OK;
    /*
     * Wait until I2C bus is free.
     */
	while(I2C1->SR2 & I2C_SR2_BUSY);

    /*
     * Generate START condition.
     */
    I2C1->CR1 |= I2C_CR1_START;

    /*
     * Wait until START condition is generated.
     * SB = Start Bit flag.
     */
    while(!(I2C1->SR1 & I2C_SR1_SB));

    /*
     * Send AT24C256 slave address with write bit.
     * 7-bit address = 0x50
     * Write byte    = 0xA0
     */
    I2C1->DR = AT24C256_ADDR_WRITE;

    /*
	 * Wait for address ACK.
	 */
	while(!(I2C1->SR1 & I2C_SR1_ADDR))
	{
		if(I2C1->SR1 & I2C_SR1_AF)
		{
			I2C1->SR1 &= ~I2C_SR1_AF;
			I2C1->CR1 |= I2C_CR1_STOP;
			return;
		}
	}

    /*
     * Clear ADDR flag by reading SR1 then SR2.
     */
    I2C1_ClearADDR();

    /*
     * Send memory address high byte.
     * AT24C256 uses 16-bit memory address.
     */
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = (uint8_t)(mem_addr >> 8);

    /*
     * Send memory address low byte.
     */
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = (uint8_t)(mem_addr & 0xFF);

    /*
     * Send data byte.
     */
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = data;

    /*
     * Wait until byte transfer is fully completed.
     * BTF = Byte Transfer Finished.
     */
	while(!(I2C1->SR1 & I2C_SR1_BTF))
	{
		if(I2C1->SR1 & I2C_SR1_AF)
		{
			I2C1->SR1 &= ~I2C_SR1_AF;
			I2C1->CR1 |= I2C_CR1_STOP;
			return;
		}
	}


    /*
     * Generate STOP condition.
     * EEPROM starts internal write cycle after STOP.
     */
    I2C1->CR1 |= I2C_CR1_STOP;

    /*
     * AT24C256 needs internal write cycle time.
     * Later we will replace this with ACK polling.
     */
    HAL_Delay(5);

    return write_stat = E_OK;
}

/*
 * START
 * 0xA0 ACK
 * memory address high byte ACK
 * memory address low byte ACK
 * REPEATED START
 * 0xA1 ACK
 * data byte NACK
 * STOP
*/

uint8_t AT24C256_ReadByte(uint16_t mem_addr, uint8_t *read_data)
{
    uint8_t read_stat = E_NOT_OK;

    /*
     * Wait until I2C bus is free.
     */
    while(I2C1->SR2 & I2C_SR2_BUSY);

    /*
     * Generate START condition.
     */
    I2C1->CR1 |= I2C_CR1_START;

    /*
     * Wait until START is generated.
     */
    while(!(I2C1->SR1 & I2C_SR1_SB));

    /*
     * Send AT24C256 address with write bit.
     * This is a dummy write phase used to load memory address.
     */
    I2C1->DR = AT24C256_ADDR_WRITE;

    /*
     * Wait until address phase is complete.
     */
    while(!(I2C1->SR1 & I2C_SR1_ADDR));

    /*
     * Clear ADDR flag.
     */
    I2C1_ClearADDR();

    /*
     * Send memory address high byte.
     */
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = (uint8_t)(mem_addr >> 8);

    /*
     * Send memory address low byte.
     */
    while(!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = (uint8_t)(mem_addr & 0xFF);

    /*
     * Wait until memory address bytes are fully transferred.
     */
    while(!(I2C1->SR1 & I2C_SR1_BTF));

    /*
     * For single byte read, disable ACK before clearing ADDR
     * in the read phase.
     */
    I2C1->CR1 &= ~I2C_CR1_ACK;

    /*
     * Generate repeated START.
     */
    I2C1->CR1 |= I2C_CR1_START;

    /*
     * Wait until repeated START is generated.
     */
    while(!(I2C1->SR1 & I2C_SR1_SB));

    /*
     * Send AT24C256 address with read bit.
     * Read byte = 0xA1.
     */
    I2C1->DR = AT24C256_ADDR_READ;

    /*
     * Wait until address phase is complete.
     */
    while(!(I2C1->SR1 & I2C_SR1_ADDR));

    /*
     * Clear ADDR flag.
     */
    I2C1_ClearADDR();

    /*
     * Generate STOP.
     * Since ACK is disabled, master will send NACK
     * after receiving this one byte.
     */
    I2C1->CR1 |= I2C_CR1_STOP;

    /*
     * Wait until received byte is available.
     */
    while(!(I2C1->SR1 & I2C_SR1_RXNE));

    /*
     * Read received byte.
     */
    *read_data = (uint8_t)I2C1->DR;

    /*
     * Re-enable ACK for future operations.
     */
    I2C1->CR1 |= I2C_CR1_ACK;

    return read_stat = E_OK;
}
