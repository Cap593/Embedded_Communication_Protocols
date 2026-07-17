/*
 * at24c256.c
 *
 *  Created on: 07-Jul-2026
 *      Author: HP
 */

/*
 * START
 * 0xA0 ACK
 * memory address high byte ACK
 * memory address low byte ACK
 * data byte ACK
 * STOP
*/

#include "at24c256.h"
#include "i2c.h"
#include "common.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"

uint8_t AT24C256_WriteByte(uint16_t mem_addr, uint8_t data)
{
    uint8_t tx_buf[3];

    if(mem_addr > AT24C256_LAST_ADDR)
    {
        return E_NOT_OK;
    }

    /*
     * AT24C256 requires 16-bit memory address.
     */
    tx_buf[0] = (uint8_t)(mem_addr >> 8);
    tx_buf[1] = (uint8_t)(mem_addr & 0xFFU);
    tx_buf[2] = data;

    if(I2C1_MasterWrite(AT24C256_I2C_ADDR_7BIT,
                        tx_buf,
                        3U) != E_OK)
    {
        return E_NOT_OK;
    }

    /*
     * EEPROM internal write cycle.
     * Later replace with ACK polling.
     */
    HAL_Delay(5);

    return E_OK;
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
    uint8_t addr_buf[2];

    if((read_data == 0) || (mem_addr > AT24C256_LAST_ADDR))
    {
        return E_NOT_OK;
    }

    addr_buf[0] = (uint8_t)(mem_addr >> 8);
    addr_buf[1] = (uint8_t)(mem_addr & 0xFFU);

    return I2C1_MasterWriteRead(AT24C256_I2C_ADDR_7BIT,
                                addr_buf,
                                2U,
                                read_data,
                                1U);
}

uint8_t AT24C256_WritePage(uint16_t mem_addr,
                           const uint8_t *data,
                           uint16_t len)
{
    uint8_t tx_buf[AT24C256_PAGE_SIZE + 2]; //for address plus data
    uint16_t i;
    uint8_t offset;
    uint8_t page_space;

    /*
     * Basic parameter checks.
     */
    if((data == NULL) || (len == 0U) || (len > AT24C256_PAGE_SIZE))
    {
        return E_NOT_OK;
    }

    /*
     * Check EEPROM address range.
     */
    if(mem_addr > AT24C256_LAST_ADDR)
    {
        return E_NOT_OK;
    }

    /*
     * Check that final written byte is still inside EEPROM.
     */
    if((mem_addr + len - 1U) > AT24C256_LAST_ADDR)
    {
        return E_NOT_OK;
    }

    /*
     * Page boundary check.
     *
     * AT24C256 page size = 64 bytes.
     * Lower 6 bits decide offset inside current page.
     */
    offset = PAGE_OFFSET_EEPROM(mem_addr);
    page_space = AT24C256_PAGE_SIZE - offset;

    /*
     * This strict WritePage API must not cross page boundary.
     */
    if(len > page_space)
    {
        return E_NOT_OK;
    }

    /*
     * AT24C256 requires 16-bit memory address before data bytes.
     */
    tx_buf[0] = (uint8_t)(mem_addr >> 8);
    tx_buf[1] = (uint8_t)(mem_addr & 0xFFU);

    /*
     * Copy page data after memory address bytes.
     */
    for(i = 0U; i < len; i++)
    {
        tx_buf[i + 2U] = data[i];
    }

    /*
     * Send:
     * START
     * Device address + Write
     * Memory address high byte
     * Memory address low byte
     * Data bytes
     * STOP
     */
    if(I2C1_MasterWrite(AT24C256_I2C_ADDR_7BIT,
                        tx_buf,
                        len + 2U) != E_OK)
    {
        return E_NOT_OK;
    }

    /*
     * EEPROM internal write cycle.
     * Later we can replace this with ACK polling.
     */
    HAL_Delay(5);

    return E_OK;
}

uint8_t AT24C256_ReadBuffer(uint16_t mem_addr,
                            uint8_t *data,
                            uint16_t len)
{
    uint8_t addr_buf[2];

    /*
     * Basic parameter checks.
     */
    if((data == NULL) || (len == 0U))
    {
        return E_NOT_OK;
    }

    /*
     * Start address must be inside EEPROM.
     */
    if(mem_addr > AT24C256_LAST_ADDR)
    {
        return E_NOT_OK;
    }

    /*
     * Final read address must also be inside EEPROM.
     */
    if((mem_addr + len - 1U) > AT24C256_LAST_ADDR)
    {
        return E_NOT_OK;
    }

    /*
     * AT24C256 uses 16-bit memory address.
     */
    addr_buf[0] = (uint8_t)(mem_addr >> 8);
    addr_buf[1] = (uint8_t)(mem_addr & 0xFFU);

    /*
     * Generic I2C write-read:
     *
     * Write phase:
     *   memory address high byte
     *   memory address low byte
     *
     * Read phase:
     *   read len bytes sequentially
     */
    if(I2C1_MasterWriteRead(AT24C256_I2C_ADDR_7BIT,
                            addr_buf,
                            2U,
                            data,
                            len) != E_OK)
    {
        return E_NOT_OK;
    }

    return E_OK;
}

uint8_t AT24C256_WriteBuffer(uint16_t mem_addr,
                             const uint8_t *data,
                             uint16_t len)
{
    uint16_t bytes_remaining;
    uint16_t current_addr;
    uint16_t data_index;
    uint16_t page_offset;
    uint16_t page_space;
    uint16_t chunk_size;

    /*
     * Basic parameter checks.
     */
    if((data == NULL) || (len == 0U))
    {
        return E_NOT_OK;
    }

    /*
     * Start address must be inside EEPROM.
     */
    if(mem_addr > AT24C256_LAST_ADDR)
    {
        return E_NOT_OK;
    }

    /*
     * Final written address must also be inside EEPROM.
     */
    if((mem_addr + len - 1U) > AT24C256_LAST_ADDR)
    {
        return E_NOT_OK;
    }

    bytes_remaining = len;
    current_addr = mem_addr;
    data_index = 0U;

    while(bytes_remaining > 0U)
    {
        /*
         * Find current offset inside 64-byte EEPROM page.
         */
        page_offset = PAGE_OFFSET_EEPROM(current_addr);

        /*
         * Number of bytes available in current page.
         */
        page_space = AT24C256_PAGE_SIZE - page_offset;

        /*
         * Choose how much to write in this transaction.
         * It must not cross page boundary.
         */
        if(bytes_remaining > page_space)
        {
            chunk_size = page_space;
        }
        else
        {
            chunk_size = bytes_remaining;
        }

        /*
         * Write one page-safe chunk.
         */
        if(AT24C256_WritePage(current_addr,
                              &data[data_index],
                              chunk_size) != E_OK)
        {
            return E_NOT_OK;
        }

        /*
         * Move to next chunk.
         */
        current_addr += chunk_size;
        data_index += chunk_size;
        bytes_remaining -= chunk_size;
    }

    return E_OK;
}


uint8_t AT24C256_Fill(uint8_t value)
{
    uint8_t page_buf[AT24C256_PAGE_SIZE];
    uint16_t i;
    uint16_t addr;

    /*
     * Prepare one page buffer filled with selected value.
     */
    for(i = 0U; i < AT24C256_PAGE_SIZE; i++)
    {
        page_buf[i] = value;
    }

    /*
     * Write the same page buffer to every EEPROM page.
     */
    for(addr = 0U; addr <= AT24C256_LAST_ADDR; addr += AT24C256_PAGE_SIZE)
    {
        if(AT24C256_WritePage(addr,
                              page_buf,
                              AT24C256_PAGE_SIZE) != E_OK)
        {
            return E_NOT_OK;
        }
    }

    return E_OK;
}

void AT24C256_TestAllApis(void)
{
    char msg[150];

    uint8_t status;
    uint8_t read_byte = 0;

    /*
     * Test addresses chosen safely away from page boundary confusion.
     */
    uint16_t byte_test_addr        = 0x0010;
    uint16_t page_test_addr        = 0x0040;  /* Page-aligned address */
    uint16_t buffer_test_addr      = 0x003C;  /* Intentionally crosses page boundary */

    /*
     * Test buffers
     */
    uint8_t page_write_data[16];
    uint8_t page_read_data[16];

    uint8_t buffer_write_data[100];
    uint8_t buffer_read_data[100];

    uint8_t i;
    uint16_t j;
    uint8_t test_pass = 1;

    USART2_SendString("\r\n========== AT24C256 EEPROM API TEST START ==========\r\n");

    /************************************************************
     * TEST 1: WriteByte + ReadByte
     ************************************************************/
    USART2_SendString("\r\n[TEST 1] WriteByte + ReadByte\r\n");

    status = AT24C256_WriteByte(byte_test_addr, 0x5A);

    if(status != E_OK)
    {
        USART2_SendString("WriteByte failed\r\n");
        test_pass = 0;
    }
    else
    {
        status = AT24C256_ReadByte(byte_test_addr, &read_byte);

        if(status != E_OK)
        {
            USART2_SendString("ReadByte failed\r\n");
            test_pass = 0;
        }
        else
        {
            sprintf(msg,
                    "ReadByte: Addr=0x%04X, Data=0x%02X\r\n",
                    byte_test_addr,
                    read_byte);

            USART2_SendString(msg);

            if(read_byte == 0x5A)
            {
                USART2_SendString("TEST 1 PASS\r\n");
            }
            else
            {
                USART2_SendString("TEST 1 FAIL: Data mismatch\r\n");
                test_pass = 0;
            }
        }
    }

    /************************************************************
     * TEST 2: WritePage + ReadBuffer
     ************************************************************/
    USART2_SendString("\r\n[TEST 2] WritePage + ReadBuffer\r\n");

    for(i = 0; i < 16; i++)
    {
        page_write_data[i] = (uint8_t)(0xA0 + i);
        page_read_data[i] = 0;
    }

    status = AT24C256_WritePage(page_test_addr,
                                page_write_data,
                                16);

    if(status != E_OK)
    {
        USART2_SendString("WritePage failed\r\n");
        test_pass = 0;
    }
    else
    {
        status = AT24C256_ReadBuffer(page_test_addr,
                                     page_read_data,
                                     16);

        if(status != E_OK)
        {
            USART2_SendString("ReadBuffer after WritePage failed\r\n");
            test_pass = 0;
        }
        else
        {
            uint8_t mismatch = 0;

            for(i = 0; i < 16; i++)
            {
                if(page_read_data[i] != page_write_data[i])
                {
                    mismatch = 1;
                    break;
                }
            }

            if(mismatch == 0)
            {
                USART2_SendString("TEST 2 PASS\r\n");
            }
            else
            {
                sprintf(msg,
                        "TEST 2 FAIL: Mismatch at index %u, W=0x%02X, R=0x%02X\r\n",
                        i,
                        page_write_data[i],
                        page_read_data[i]);

                USART2_SendString(msg);
                test_pass = 0;
            }
        }
    }

    /************************************************************
     * TEST 3: WriteBuffer across page boundary + ReadBuffer
     ************************************************************/
    USART2_SendString("\r\n[TEST 3] WriteBuffer across page boundary + ReadBuffer\r\n");

    /*
     * buffer_test_addr = 0x003C
     * This crosses page boundary:
     * 0x003C to 0x003F = 4 bytes in current page
     * remaining bytes continue from 0x0040 onwards
     */
    for(j = 0; j < 100; j++)
    {
        buffer_write_data[j] = (uint8_t)(j + 1U);
        buffer_read_data[j] = 0;
    }

    status = AT24C256_WriteBuffer(buffer_test_addr,
                                  buffer_write_data,
                                  100);

    if(status != E_OK)
    {
        USART2_SendString("WriteBuffer failed\r\n");
        test_pass = 0;
    }
    else
    {
        status = AT24C256_ReadBuffer(buffer_test_addr,
                                     buffer_read_data,
                                     100);

        if(status != E_OK)
        {
            USART2_SendString("ReadBuffer after WriteBuffer failed\r\n");
            test_pass = 0;
        }
        else
        {
            uint8_t mismatch = 0;
            uint16_t mismatch_index = 0;

            for(j = 0; j < 100; j++)
            {
                if(buffer_read_data[j] != buffer_write_data[j])
                {
                    mismatch = 1;
                    mismatch_index = j;
                    break;
                }
            }

            if(mismatch == 0)
            {
                USART2_SendString("TEST 3 PASS\r\n");
            }
            else
            {
                sprintf(msg,
                        "TEST 3 FAIL: Mismatch at index %u, W=0x%02X, R=0x%02X\r\n",
                        mismatch_index,
                        buffer_write_data[mismatch_index],
                        buffer_read_data[mismatch_index]);

                USART2_SendString(msg);
                test_pass = 0;
            }
        }
    }

    /************************************************************
     * TEST 4: Fill EEPROM with value and verify small region
     ************************************************************/
    USART2_SendString("\r\n[TEST 4] Fill EEPROM with 0xFF and verify first 32 bytes\r\n");

    status = AT24C256_Fill(0xFF);

    if(status != E_OK)
    {
        USART2_SendString("Fill failed\r\n");
        test_pass = 0;
    }
    else
    {
        uint8_t fill_read_data[32];
        uint8_t mismatch = 0;

        memset(fill_read_data, 0, sizeof(fill_read_data));

        status = AT24C256_ReadBuffer(0x0000,
                                     fill_read_data,
                                     32);

        if(status != E_OK)
        {
            USART2_SendString("ReadBuffer after Fill failed\r\n");
            test_pass = 0;
        }
        else
        {
            for(i = 0; i < 32; i++)
            {
                if(fill_read_data[i] != 0xFF)
                {
                    mismatch = 1;
                    break;
                }
            }

            if(mismatch == 0)
            {
                USART2_SendString("TEST 4 PASS\r\n");
            }
            else
            {
                sprintf(msg,
                        "TEST 4 FAIL: Mismatch at index %u, R=0x%02X\r\n",
                        i,
                        fill_read_data[i]);

                USART2_SendString(msg);
                test_pass = 0;
            }
        }
    }

    /************************************************************
     * Final result
     ************************************************************/
    USART2_SendString("\r\n========== AT24C256 EEPROM API TEST RESULT ==========\r\n");

    if(test_pass)
    {
        USART2_SendString("ALL TESTS PASS\r\n");
    }
    else
    {
        USART2_SendString("ONE OR MORE TESTS FAILED\r\n");
    }

    USART2_SendString("========== AT24C256 EEPROM API TEST END ==========\r\n\r\n");
}

