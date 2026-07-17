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



void I2C1_GPIO_Init(void)
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

void I2C1_Init_100kHz(void)
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
    I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;

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
    I2C1->CR2 &= ~I2C_CR2_FREQ;
    I2C1->CR2 |= 32U;

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
    I2C1->CCR = 160U;

    /*
     * Configure maximum rise time.
     *
     * For standard mode:
     * TRISE = FREQ + 1
     * TRISE = 32 + 1 = 33
     */
    I2C1->TRISE = 33U;


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

uint8_t I2C1_MasterWrite(uint8_t slave_addr_7bit,
                         const uint8_t *tx_data,
                         uint16_t len)
{
    uint16_t i;

    if((tx_data == 0) || (len == 0U))
    {
        return E_NOT_OK;
    }

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
     * Send slave address + write bit.
     */
    I2C1->DR = (uint8_t)(slave_addr_7bit << 1U);

    while(!(I2C1->SR1 & I2C_SR1_ADDR))
    {
        if(I2C1->SR1 & I2C_SR1_AF)
        {
            I2C1->SR1 &= ~I2C_SR1_AF;
            I2C1->CR1 |= I2C_CR1_STOP;
            return E_NOT_OK;
        }
    }

    /*
     * Clear ADDR flag by reading SR1 then SR2.
     */
    I2C1_ClearADDR();

    /*
	 * Send data byte.
	 */
    for(i = 0U; i < len; i++)
    {
        while(!(I2C1->SR1 & I2C_SR1_TXE));

        I2C1->DR = tx_data[i];

        if(I2C1->SR1 & I2C_SR1_AF)
        {
            I2C1->SR1 &= ~I2C_SR1_AF;
            I2C1->CR1 |= I2C_CR1_STOP;
            return E_NOT_OK;
        }
    }

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
            return E_NOT_OK;
        }
    }

    I2C1->CR1 |= I2C_CR1_STOP;

    return E_OK;
}



uint8_t I2C1_MasterWriteRead(uint8_t slave_addr_7bit,
                             const uint8_t *tx_data,
                             uint16_t tx_len,
                             uint8_t *rx_data,
                             uint16_t rx_len)
{
    uint16_t i;

    if((tx_data == NULL) || (tx_len == 0U) ||
       (rx_data == NULL) || (rx_len == 0U))
    {
        return E_NOT_OK;
    }

    /*
     * Wait until I2C bus is free.
     */
    while(I2C1->SR2 & I2C_SR2_BUSY);

    /************************************************************
     * WRITE PHASE
     ************************************************************/

    /*
     * Generate START.
     */
    I2C1->CR1 |= I2C_CR1_START;

    /*
     * Wait until START generated.
     */
    while(!(I2C1->SR1 & I2C_SR1_SB));

    /*
     * Send slave address + write bit.
     */
    I2C1->DR = (uint8_t)(slave_addr_7bit << 1U);

    /*
     * Wait for address ACK.
     */
    while(!(I2C1->SR1 & I2C_SR1_ADDR))
    {
        if(I2C1->SR1 & I2C_SR1_AF)
        {
            I2C1->SR1 &= ~I2C_SR1_AF;
            I2C1->CR1 |= I2C_CR1_STOP;
            return E_NOT_OK;
        }
    }

    /*
     * Clear ADDR.
     */
    I2C1_ClearADDR();

    /*
     * Send tx bytes.
     * For AT24C256 random read, this is memory address high + low.
     */
    for(i = 0U; i < tx_len; i++)
    {
        while(!(I2C1->SR1 & I2C_SR1_TXE));

        I2C1->DR = tx_data[i];
    }

    /*
     * Wait until write phase fully completed.
     */
    while(!(I2C1->SR1 & I2C_SR1_BTF));

    /************************************************************
     * READ PHASE
     ************************************************************/

    /*
     * Generate repeated START.
     */
    I2C1->CR1 |= I2C_CR1_START;

    /*
     * Wait until repeated START generated.
     */
    while(!(I2C1->SR1 & I2C_SR1_SB));

    /*
     * Send slave address + read bit.
     */
    I2C1->DR = (uint8_t)((slave_addr_7bit << 1U) | 1U);

    /*
     * Wait for address ACK.
     */
    while(!(I2C1->SR1 & I2C_SR1_ADDR))
    {
        if(I2C1->SR1 & I2C_SR1_AF)
        {
            I2C1->SR1 &= ~I2C_SR1_AF;
            I2C1->CR1 |= I2C_CR1_STOP;
            return E_NOT_OK;
        }
    }

    /************************************************************
     * CASE 1: Single-byte read
     ************************************************************/
    if(rx_len == 1U)
    {
        /*
         * For single-byte read:
         * Disable ACK before clearing ADDR.
         */
        I2C1->CR1 &= ~I2C_CR1_ACK;

        /*
         * Clear ADDR.
         */
        I2C1_ClearADDR();

        /*
         * Generate STOP.
         * Hardware will NACK the received byte and then STOP.
         */
        I2C1->CR1 |= I2C_CR1_STOP;

        /*
         * Wait until data received.
         */
        while(!(I2C1->SR1 & I2C_SR1_RXNE));

        /*
         * Read data.
         */
        rx_data[0] = (uint8_t)I2C1->DR;

        /*
         * Re-enable ACK for future operations.
         */
        I2C1->CR1 |= I2C_CR1_ACK;
    }

    /************************************************************
     * CASE 2: Two-byte read
     ************************************************************/
    else if(rx_len == 2U)
    {
        /*
         * For 2-byte read:
         * Set POS before clearing ADDR.
         * Disable ACK before clearing ADDR.
         */
        I2C1->CR1 |= I2C_CR1_POS;
        I2C1->CR1 &= ~I2C_CR1_ACK;

        /*
         * Clear ADDR.
         */
        I2C1_ClearADDR();

        /*
         * Wait until both bytes are received.
         * BTF = 1 means two bytes are ready:
         * one in DR, one in shift register.
         */
        while(!(I2C1->SR1 & I2C_SR1_BTF));

        /*
         * Generate STOP before reading the two bytes.
         */
        I2C1->CR1 |= I2C_CR1_STOP;

        /*
         * Read first byte.
         */
        rx_data[0] = (uint8_t)I2C1->DR;

        /*
         * Read second byte.
         */
        rx_data[1] = (uint8_t)I2C1->DR;

        /*
         * Restore ACK and POS for future operations.
         */
        I2C1->CR1 &= ~I2C_CR1_POS;
        I2C1->CR1 |= I2C_CR1_ACK;
    }

    /************************************************************
     * CASE 3: More than two bytes
     ************************************************************/
    else
    {
        uint16_t remaining = rx_len;

        /*
         * Enable ACK for multi-byte reception.
         */
        I2C1->CR1 |= I2C_CR1_ACK;

        /*
         * Clear ADDR.
         */
        I2C1_ClearADDR();

        /*
         * Receive bytes until only 3 bytes remain.
         */
        while(remaining > 3U)
        {
            while(!(I2C1->SR1 & I2C_SR1_RXNE));

            *rx_data = (uint8_t)I2C1->DR;
            rx_data++;
            remaining--;
        }

        /*
         * Now exactly 3 bytes are remaining.
         * Wait until BTF = 1.
         */
        while(!(I2C1->SR1 & I2C_SR1_BTF));

        /*
         * Disable ACK.
         * This prepares NACK for the final byte.
         */
        I2C1->CR1 &= ~I2C_CR1_ACK;

        /*
         * Read N-2 byte.
         */
        *rx_data = (uint8_t)I2C1->DR;
        rx_data++;
        remaining--;

        /*
         * Wait until BTF = 1 again.
         * Now last two bytes are ready.
         */
        while(!(I2C1->SR1 & I2C_SR1_BTF));

        /*
         * Generate STOP before reading final two bytes.
         */
        I2C1->CR1 |= I2C_CR1_STOP;

        /*
         * Read N-1 byte.
         */
        *rx_data = (uint8_t)I2C1->DR;
        rx_data++;
        remaining--;

        /*
         * Read N byte.
         */
        *rx_data = (uint8_t)I2C1->DR;
        remaining--;

        /*
         * Re-enable ACK for future operations.
         */
        I2C1->CR1 |= I2C_CR1_ACK;
    }

    return E_OK;
}


