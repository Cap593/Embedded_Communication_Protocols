/*
 * ads1115.c
 *
 *  Created on: 09-Jul-2026
 *      Author: HP
 */

#include "ads1115.h"
#include "i2c.h"
#include "usart.h"
#include "stdio.h"
#include "common.h"

static void ADS1115_PrintVoltage(int16_t raw, float voltage);

static void ADS1115_PrintVoltage(int16_t raw, float voltage)
{
    char msg[100];
    int32_t voltage_mV;
    int32_t voltage_int;
    int32_t voltage_frac;

    /*
     * For single-ended input, small negative values near 0V
     * can appear due to offset/noise. Clamp only for display.
     */
    if(voltage < 0.0f)
    {
        voltage = 0.0f;
    }

    /*
     * Convert voltage to millivolts.
     * Example:
     * 3.313V -> 3313 mV
     */
    voltage_mV = (int32_t)((voltage * 1000.0f) + 0.5f);

    voltage_int  = voltage_mV / 1000;
    voltage_frac = voltage_mV % 1000;

    sprintf(msg,
            "AIN0 Raw = %d, Voltage = %ld.%03ldV\r\n",
            raw,
            voltage_int,
            voltage_frac);

    USART2_SendString(msg);
}

uint8_t ADS1115_Init(void)
{
    uint16_t config_value;

    /*
     * After power-up, ADS1115 should respond on I2C.
     * Read Config register to verify communication.
     */
    if(ADS1115_ReadRegister(ADS1115_ADDR_GND,
                            ADS1115_REG_CONFIG,
                            &config_value) != E_OK)
    {
        return E_NOT_OK;
    }

    /*
     * Optional: verify reset/default value.
     *
     * The default Config register value is 0x8583.
     * But do not make init fail only because of mismatch yet,
     * because if code previously configured the ADC and power was
     * not cycled, the value may be different.
     */

    if(config_value == ADS1115_CONFIG_RESET_VALUE)
    {
        USART2_SendString("ADS1115 Init OK: default config detected\r\n");
    }
    else
    {
        USART2_SendString("ADS1115 Init OK: config differs from reset value\r\n");
    }

    /*
     * For first driver stage, we only confirm communication.
     * Later, we will write our preferred default config here.
     */

    return E_OK;
}


uint8_t ADS1115_WriteRegister(uint8_t dev_addr,
                              uint8_t reg_addr,
                              uint16_t reg_data)
{
    uint8_t tx_data[3];

    /*
     * ADS1115 register write frame:
     *
     * START
     * device address + W
     * register pointer
     * register MSB
     * register LSB
     * STOP
     */
    tx_data[0] = reg_addr;
    tx_data[1] = (uint8_t)((reg_data >> 8) & 0xFFU);
    tx_data[2] = (uint8_t)(reg_data & 0xFFU);

    if(I2C1_MasterWrite(dev_addr, tx_data, 3U) != E_OK)
    {
        USART2_SendString("ADS1115 Write Failed...\r\n");
        return E_NOT_OK;
    }

    return E_OK;
}


uint8_t ADS1115_ReadRegister(uint8_t dev_addr,
                             uint8_t reg_addr,
                             uint16_t *reg_data)
{
    uint8_t rx_data[2];

    if(reg_data == NULL)
    {
        return E_NOT_OK;
    }

    /*
     * ADS1115 register read frame:
     *
     * Write phase:
     *   register pointer
     *
     * Read phase:
     *   register MSB
     *   register LSB
     */
    if(I2C1_MasterWriteRead(dev_addr,
                            &reg_addr,
                            1U,
                            rx_data,
                            2U) != E_OK)
    {
        USART2_SendString("ADS1115 Read Failed...\r\n");
        return E_NOT_OK;
    }

    /*
     * ADS1115 sends MSB first, then LSB.
     */
    *reg_data = ((uint16_t)rx_data[0] << 8) | rx_data[1];

    return E_OK;
}

void ADS1115_TestBasic(void)
{
    uint16_t config;
    uint16_t conversion;
    char msg[100];

    if(ADS1115_Init() != E_OK)
    {
        USART2_SendString("ADS1115 Basic Test Failed: Init error\r\n");
        return;
    }

    if(ADS1115_ReadRegister(ADS1115_ADDR_GND,
                            ADS1115_REG_CONFIG,
                            &config) == E_OK)
    {
        sprintf(msg, "ADS1115 Config Register = 0x%04X\r\n", config);
        USART2_SendString(msg);
    }
    else
    {
        USART2_SendString("ADS1115 Config Register Read Failed\r\n");
    }

    if(ADS1115_ReadRegister(ADS1115_ADDR_GND,
                            ADS1115_REG_CONVERSION,
                            &conversion) == E_OK)
    {
        sprintf(msg, "ADS1115 Conversion Register = 0x%04X\r\n", conversion);
        USART2_SendString(msg);
    }
    else
    {
        USART2_SendString("ADS1115 Conversion Register Read Failed\r\n");
    }
}

uint8_t ADS1115_StartSingleShot_AIN0(void)
{
    return ADS1115_WriteRegister(ADS1115_ADDR_GND,
                                 ADS1115_REG_CONFIG,
                                 ADS1115_CONFIG_AIN0_SINGLE_4V096_128SPS);
}

uint8_t ADS1115_ReadRaw(int16_t *raw)
{
    uint16_t reg_value;

    if(raw == NULL)
    {
        return E_NOT_OK;
    }

    if(ADS1115_ReadRegister(ADS1115_ADDR_GND,
                            ADS1115_REG_CONVERSION,
                            &reg_value) != E_OK)
    {
        return E_NOT_OK;
    }

    *raw = (int16_t)reg_value;

    return E_OK;
}

uint8_t ADS1115_IsConversionReady(uint8_t *ready)
{
    uint16_t config;

    if(ready == NULL)
    {
        return E_NOT_OK;
    }

    if(ADS1115_ReadRegister(ADS1115_ADDR_GND,
                            ADS1115_REG_CONFIG,
                            &config) != E_OK)
    {
        return E_NOT_OK;
    }

    if(config & 0x8000U)
    {
        *ready = 1U;
    }
    else
    {
        *ready = 0U;
    }

    return E_OK;
}

float ADS1115_RawToVoltage(int16_t raw)
{
    /*
     * PGA = ±4.096 V
     * LSB = 4.096 / 32768 = 0.000125 V
     */
    return ((float)raw * 0.000125f);
}

uint8_t ADS1115_ReadSingleEnded_AIN0(int16_t *raw, float *voltage)
{
    uint8_t ready = 0U;

    if((raw == NULL) || (voltage == NULL))
    {
        return E_NOT_OK;
    }

    if(ADS1115_StartSingleShot_AIN0() != E_OK)
    {
        return E_NOT_OK;
    }

    /*
     * At 128 SPS, conversion time is about 7.8 ms.
     * We poll OS bit here.
     */
    do
    {
        if(ADS1115_IsConversionReady(&ready) != E_OK)
        {
            return E_NOT_OK;
        }

    } while(ready == 0U);

    if(ADS1115_ReadRaw(raw) != E_OK)
    {
        return E_NOT_OK;
    }

    *voltage = ADS1115_RawToVoltage(*raw);

    return E_OK;
}

void ADS1115_Test_AIN0(void)
{
    int16_t raw;
    float voltage;

    if(ADS1115_ReadSingleEnded_AIN0(&raw, &voltage) == E_OK)
    {
        ADS1115_PrintVoltage(raw, voltage);
    }
    else
    {
        USART2_SendString("ADS1115 AIN0 Read Failed\r\n");
    }
}

uint8_t ADS1115_StartContinuous_AIN0(void)
{
    /*
     * Configure ADS1115:
     * AIN0 single-ended
     * PGA = ±4.096V
     * Continuous conversion mode
     * Data rate = 128SPS
     * Comparator disabled
     */
    return ADS1115_WriteRegister(ADS1115_ADDR_GND,
                                 ADS1115_REG_CONFIG,
                                 ADS1115_CONFIG_AIN0_CONT_4V096_128SPS);
}

uint8_t ADS1115_StopContinuous(void)
{
    /*
     * MODE = 1 puts device back into single-shot / power-down mode.
     * This config keeps AIN0, PGA ±4.096V, 128SPS, comparator disabled.
     */
    return ADS1115_WriteRegister(ADS1115_ADDR_GND,
                                 ADS1115_REG_CONFIG,
                                 ADS1115_CONFIG_AIN0_SINGLE_4V096_128SPS);
}

uint8_t ADS1115_ReadContinuousRaw(int16_t *raw)
{
    uint16_t reg_value;

    if(raw == NULL)
    {
        return E_NOT_OK;
    }

    if(ADS1115_ReadRegister(ADS1115_ADDR_GND,
                            ADS1115_REG_CONVERSION,
                            &reg_value) != E_OK)
    {
        return E_NOT_OK;
    }

    /*
     * ADS1115 conversion register is signed two's complement.
     */
    *raw = (int16_t)reg_value;

    return E_OK;
}

uint8_t ADS1115_RawToMilliVolt_4V096(int16_t raw,
                                     uint16_t *voltage_mV)
{
    int32_t voltage_uV;

    if(voltage_mV == NULL)
    {
        return E_NOT_OK;
    }

    /*
     * Single-ended mode:
     * Clamp small negative values caused by offset/noise.
     */
    if(raw < 0)
    {
        raw = 0;
    }

    voltage_uV = (int32_t)raw * ADS1115_LSB_4V096_UV;

    /*
     * Convert µV to mV with rounding.
     */
    *voltage_mV = (uint16_t)((voltage_uV + 500) / 1000);

    return E_OK;
}

uint8_t ADS1115_ReadContinuous_AIN0(int16_t *raw,
                                    uint16_t *voltage_mV)
{
    if((raw == NULL) || (voltage_mV == NULL))
    {
        return E_NOT_OK;
    }

    if(ADS1115_ReadContinuousRaw(raw) != E_OK)
    {
        return E_NOT_OK;
    }

    if(ADS1115_RawToMilliVolt_4V096(*raw, voltage_mV) != E_OK)
    {
        return E_NOT_OK;
    }

    return E_OK;
}

void ADS1115_TestContinuous_AIN0(void)
{
    int16_t raw;
    uint16_t voltage_mV;
    char msg[100];

    if(ADS1115_StartContinuous_AIN0() != E_OK)
    {
        USART2_SendString("ADS1115 continuous start failed\r\n");
        return;
    }

    USART2_SendString("ADS1115 continuous mode started\r\n");

    /*
     * At 128 SPS, one conversion takes about 7.8 ms.
     * Wait once before reading first stable result.
     */
    HAL_Delay(10);

    while(1)
    {
        if(ADS1115_ReadContinuous_AIN0(&raw, &voltage_mV) == E_OK)
        {
            sprintf(msg,
                    "CONT AIN0 Raw = %d, Voltage = %u.%03uV\r\n",
                    raw,
                    voltage_mV / 1000U,
                    voltage_mV % 1000U);

            USART2_SendString(msg);
        }
        else
        {
            USART2_SendString("ADS1115 continuous read failed\r\n");
        }

        /*
         * Do not read faster than needed.
         * At 128SPS, new data appears every ~7.8ms.
         * 100ms is fine for terminal display.
         */
        HAL_Delay(100);
    }
}
