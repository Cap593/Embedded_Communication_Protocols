#include "w25q64fv.h"
#include "spi.h"
#include "common.h"
#include <string.h>

/* Safe RAM buffer used by sector read-modify-erase-write logic */
static uint8_t sector_buffer[W25Q_SECTOR_SIZE];
uint8_t block_buffer[W25Q_BLOCK64_SIZE];

static W25Q_Status_t W25Q_CheckRange(uint32_t addr, uint32_t size)
{
    W25Q_Status_t status = W25Q_OK;

    if (size == 0U)
    {
        status = W25Q_INVALID_PARAM;
    }
    else if (addr >= W25Q_CHIP_SIZE)
    {
        status = W25Q_INVALID_PARAM;
    }
    else if (size > (W25Q_CHIP_SIZE - addr))
    {
        /*
         * Avoid checking (addr + size) directly, because addr + size
         * can overflow for large values.
         */
        status = W25Q_INVALID_PARAM;
    }
    else
    {
        status = W25Q_OK;
    }

    return status;
}

void W25Q_FlashReset(void)
{
    /*
     * Software reset sequence:
     * 1. Enable Reset command: 0x66
     * 2. Reset Device command: 0x99
     *
     * These are sent as separate SPI instructions.
     */
    SPI_DeselectAll();

    W25Q_FLASH_CS_LOW();
    SPI2_TransferByte(W25Q_CMD_ENABLE_RESET);
    W25Q_FLASH_CS_HIGH();

    W25Q_FLASH_CS_LOW();
    SPI2_TransferByte(W25Q_CMD_RESET_DEVICE);
    W25Q_FLASH_CS_HIGH();

    /*
     * The device needs a small delay after reset.
     * 10 ms is conservative and safe for bring-up.
     */
    HAL_Delay(10U);
}

void W25Q_FlashReadJEDEC(uint8_t *id)
{
    if (id != NULL)
    {
        SPI_DeselectAll();

        W25Q_FLASH_CS_LOW();

        SPI2_TransferByte(W25Q_CMD_READ_JEDEC_ID);

        id[0] = SPI2_TransferByte(W25Q_DUMMY_BYTE);
        id[1] = SPI2_TransferByte(W25Q_DUMMY_BYTE);
        id[2] = SPI2_TransferByte(W25Q_DUMMY_BYTE);

        W25Q_FLASH_CS_HIGH();
    }
}

void W25Q_FlashRead_StatusRegister(uint8_t *status)
{
    if (status != NULL)
    {
        SPI_DeselectAll();

        /* Status Register 1 */
        W25Q_FLASH_CS_LOW();
        SPI2_TransferByte(W25Q_CMD_READ_STATUS_REG1);
        status[0] = SPI2_TransferByte(W25Q_DUMMY_BYTE);
        W25Q_FLASH_CS_HIGH();

        /* Status Register 2 */
        W25Q_FLASH_CS_LOW();
        SPI2_TransferByte(W25Q_CMD_READ_STATUS_REG2);
        status[1] = SPI2_TransferByte(W25Q_DUMMY_BYTE);
        W25Q_FLASH_CS_HIGH();
    }
}

uint8_t W25Q_FlashReadStatusReg1(void)
{
    uint8_t status_reg1 = 0U;

    SPI_DeselectAll();

    W25Q_FLASH_CS_LOW();
    SPI2_TransferByte(W25Q_CMD_READ_STATUS_REG1);
    status_reg1 = SPI2_TransferByte(W25Q_DUMMY_BYTE);
    W25Q_FLASH_CS_HIGH();

    return status_reg1;
}

W25Q_Status_t W25Q_FlashWaitUntilReady(void)
{
    W25Q_Status_t status = W25Q_TIMEOUT;
    uint32_t timeout = W25Q_WAIT_TIMEOUT_COUNT;

    while ((timeout > 0U) && (status == W25Q_TIMEOUT))
    {
        if ((W25Q_FlashReadStatusReg1() & W25Q_SR1_BUSY) == 0U)
        {
            status = W25Q_OK;
        }
        else
        {
            timeout--;
        }
    }

    return status;
}

W25Q_Status_t W25Q_FlashWriteEnable(void)
{
    W25Q_Status_t status = W25Q_OK;

    SPI_DeselectAll();

    W25Q_FLASH_CS_LOW();
    SPI2_TransferByte(W25Q_CMD_WRITE_ENABLE);
    W25Q_FLASH_CS_HIGH();

    if ((W25Q_FlashReadStatusReg1() & W25Q_SR1_WEL) == 0U)
    {
        status = W25Q_ERROR;
    }

    return status;
}

void W25Q_FlashWriteDisable(void)
{
    SPI_DeselectAll();

    W25Q_FLASH_CS_LOW();
    SPI2_TransferByte(W25Q_CMD_WRITE_DISABLE);
    W25Q_FLASH_CS_HIGH();
}

W25Q_Status_t W25Q_FlashRead_Data(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    W25Q_Status_t status = W25Q_OK;

    if ((buffer == NULL) || (size == 0U))
    {
        status = W25Q_INVALID_PARAM;
    }

    if (status == W25Q_OK)
    {
        status = W25Q_CheckRange(addr, size);
    }

    if (status == W25Q_OK)
    {
        SPI_DeselectAll();

        W25Q_FLASH_CS_LOW();

        SPI2_TransferByte(W25Q_CMD_READ_DATA);

        /* Send 24-bit address: A23-A16, A15-A8, A7-A0 */
        SPI2_TransferByte((uint8_t)((addr >> 16U) & 0xFFU));
        SPI2_TransferByte((uint8_t)((addr >> 8U)  & 0xFFU));
        SPI2_TransferByte((uint8_t)( addr         & 0xFFU));

        for (uint32_t i = 0U; i < size; i++)
        {
            buffer[i] = SPI2_TransferByte(W25Q_DUMMY_BYTE);
        }

        W25Q_FLASH_CS_HIGH();
    }

    return status;
}

W25Q_Status_t W25Q_FlashSectorErase(uint32_t addr)
{
    W25Q_Status_t status;
    uint32_t sector_base = 0U;

    status = W25Q_CheckRange(addr, 1U);

    if (status == W25Q_OK)
    {
        sector_base = SECTOR_BASE(addr);

        status = W25Q_FlashWriteEnable();
    }

    if (status == W25Q_OK)
    {
        SPI_DeselectAll();

        W25Q_FLASH_CS_LOW();

        SPI2_TransferByte(W25Q_CMD_SECTOR_ERASE_4KB);

        SPI2_TransferByte((uint8_t)((sector_base >> 16U) & 0xFFU));
        SPI2_TransferByte((uint8_t)((sector_base >> 8U)  & 0xFFU));
        SPI2_TransferByte((uint8_t)( sector_base         & 0xFFU));

        W25Q_FLASH_CS_HIGH();

        status = W25Q_FlashWaitUntilReady();
    }

    return status;
}

W25Q_Status_t W25Q_FlashPageProgram(uint32_t addr, uint8_t *data, uint32_t size)
{
    W25Q_Status_t status = W25Q_OK;
    uint32_t page_offset = 0U;

    if ((data == NULL) || (size == 0U))
    {
        status = W25Q_INVALID_PARAM;
    }

    if (status == W25Q_OK)
    {
        status = W25Q_CheckRange(addr, size);
    }

    if (status == W25Q_OK)
    {
        page_offset = PAGE_OFFSET(addr);

        if ((size > W25Q_PAGE_SIZE) ||
            ((page_offset + size) > W25Q_PAGE_SIZE))
        {
            status = W25Q_INVALID_PARAM;
        }
    }

    if (status == W25Q_OK)
    {
        status = W25Q_FlashWriteEnable();
    }

    if (status == W25Q_OK)
    {
        SPI_DeselectAll();

        W25Q_FLASH_CS_LOW();

        SPI2_TransferByte(W25Q_CMD_PAGE_PROGRAM);

        SPI2_TransferByte((uint8_t)((addr >> 16U) & 0xFFU));
        SPI2_TransferByte((uint8_t)((addr >> 8U)  & 0xFFU));
        SPI2_TransferByte((uint8_t)( addr         & 0xFFU));

        for (uint32_t i = 0U; i < size; i++)
        {
            SPI2_TransferByte(data[i]);
        }

        W25Q_FLASH_CS_HIGH();

        status = W25Q_FlashWaitUntilReady();
    }

    return status;
}

W25Q_Status_t W25Q_FlashWrite_NoErase(uint32_t addr, uint8_t *data, uint32_t size)
{
    W25Q_Status_t status = W25Q_OK;
    uint32_t bytes_to_write = 0U;
    uint32_t page_offset = 0U;
    uint32_t page_space = 0U;

    if ((data == NULL) || (size == 0U))
    {
        status = W25Q_INVALID_PARAM;
    }

    if (status == W25Q_OK)
    {
        status = W25Q_CheckRange(addr, size);
    }

    while ((size > 0U) && (status == W25Q_OK))
    {
        page_offset = PAGE_OFFSET(addr);
        page_space  = W25Q_PAGE_SIZE - page_offset;

        if (size < page_space)
        {
            bytes_to_write = size;
        }
        else
        {
            bytes_to_write = page_space;
        }

        status = W25Q_FlashPageProgram(addr, data, bytes_to_write);

        if (status == W25Q_OK)
        {
            addr += bytes_to_write;
            data += bytes_to_write;
            size -= bytes_to_write;
        }
    }

    return status;
}

W25Q_Status_t W25Q_FlashUpdate_SectorSafe(uint32_t addr, uint8_t *data, uint32_t size)
{
    W25Q_Status_t status = W25Q_OK;
    uint32_t sector_base = 0U;
    uint32_t sector_offset = 0U;
    uint32_t bytes_in_this_sector = 0U;

    if ((data == NULL) || (size == 0U))
    {
        status = W25Q_INVALID_PARAM;
    }

    if (status == W25Q_OK)
    {
        status = W25Q_CheckRange(addr, size);
    }

    while ((size > 0U) && (status == W25Q_OK))
    {
        sector_base   = SECTOR_BASE(addr);
        sector_offset = SECTOR_OFFSET(addr);

        bytes_in_this_sector = W25Q_SECTOR_SIZE - sector_offset;

        if (size < bytes_in_this_sector)
        {
            bytes_in_this_sector = size;
        }

        /* 1. Read full sector into RAM */
        status = W25Q_FlashRead_Data(sector_base, sector_buffer, W25Q_SECTOR_SIZE);

        /* 2. Modify RAM copy */
        if (status == W25Q_OK)
        {
            memcpy(&sector_buffer[sector_offset], data, bytes_in_this_sector);
        }

        /* 3. Erase full sector */
        if (status == W25Q_OK)
        {
            status = W25Q_FlashSectorErase(sector_base);
        }

        /* 4. Program updated sector back page-by-page */
        if (status == W25Q_OK)
        {
            status = W25Q_FlashWrite_NoErase(sector_base, sector_buffer, W25Q_SECTOR_SIZE);
        }

        if (status == W25Q_OK)
        {
            addr += bytes_in_this_sector;
            data += bytes_in_this_sector;
            size -= bytes_in_this_sector;
        }
    }

    return status;
}

W25Q_Status_t W25Q_FlashVerify(uint32_t addr, uint8_t *expected, uint32_t size)
{
    W25Q_Status_t status = W25Q_OK;
    uint8_t verify_buffer[32];
    uint32_t chunk = 0U;

    if ((expected == NULL) || (size == 0U))
    {
        status = W25Q_INVALID_PARAM;
    }

    if (status == W25Q_OK)
    {
        status = W25Q_CheckRange(addr, size);
    }

    while ((size > 0U) && (status == W25Q_OK))
    {
        if (size > sizeof(verify_buffer))
        {
            chunk = sizeof(verify_buffer);
        }
        else
        {
            chunk = size;
        }

        status = W25Q_FlashRead_Data(addr, verify_buffer, chunk);

        if (status == W25Q_OK)
        {
            if (memcmp(verify_buffer, expected, chunk) != 0)
            {
                status = W25Q_VERIFY_FAILED;
            }
        }

        if (status == W25Q_OK)
        {
            addr += chunk;
            expected += chunk;
            size -= chunk;
        }
    }

    return status;
}

W25Q_Status_t W25Q_FlashSelfTest(void)
{
    W25Q_Status_t status = W25Q_OK;

    uint8_t jedec_id[3] = {0U, 0U, 0U};
    uint8_t w25q_status[2] = {0U, 0U};

    uint8_t tx_data[5] = {'H', 'E', 'L', 'L', 'O'};
    uint8_t rx_data[64];

    uint8_t page_cross_data[48] =
    {
        0x11U, 0x22U, 0x33U, 0x44U, 0x55U, 0x66U, 0x77U, 0x88U,
        0x99U, 0xAAU, 0xBBU, 0xCCU, 0xDDU, 0xEEU, 0xFFU, 0x10U,
        0x21U, 0x22U, 0x23U, 0x24U, 0x25U, 0x26U, 0x27U, 0x28U,
        0x29U, 0x2AU, 0x2BU, 0x2CU, 0x2DU, 0x2EU, 0x2FU, 0x20U,
        0x31U, 0x32U, 0x33U, 0x34U, 0x35U, 0x36U, 0x37U, 0x38U,
        0x39U, 0x3AU, 0x3BU, 0x3CU, 0x3DU, 0x3EU, 0x3FU, 0x30U
    };

    uint8_t sector_cross_data[32];

    for (uint32_t i = 0U; i < sizeof(sector_cross_data); i++)
    {
        sector_cross_data[i] = (uint8_t)(0xA0U + i);
    }

    memset(sector_buffer, 0xFF, sizeof(sector_buffer));

    /* 1. Reset and read JEDEC */
    W25Q_FlashReset();
    W25Q_FlashReadJEDEC(jedec_id);

    if ((jedec_id[0] != W25Q_MANUFACTURER_WINBOND) ||
        (jedec_id[1] != W25Q_MEMORY_TYPE_SPI))
    {
        status = W25Q_UNKNOWN_DEVICE;
    }

    /*
     * Your board earlier returned 0x18.
     * Accept both 0x17 and 0x18 for now.
     */
    if (status == W25Q_OK)
    {
        if ((jedec_id[2] != W25Q_CAPACITY_64MBIT) &&
            (jedec_id[2] != W25Q_CAPACITY_128MBIT))
        {
            status = W25Q_UNKNOWN_DEVICE;
        }
    }

    /* 2. Read status register */
    if (status == W25Q_OK)
    {
        W25Q_FlashRead_StatusRegister(w25q_status);

        if ((w25q_status[0] & W25Q_SR1_BUSY) != 0U)
        {
            status = W25Q_ERROR;
        }
    }

    /* 3. Erase test sector and verify 0xFF */
    if (status == W25Q_OK)
    {
        status = W25Q_FlashSectorErase(W25Q_TEST_ADDR);
    }

    if (status == W25Q_OK)
    {
        status = W25Q_FlashVerify(W25Q_TEST_ADDR, sector_buffer, W25Q_SECTOR_SIZE);
    }

    /* 4. Program small data and verify */
    if (status == W25Q_OK)
    {
        status = W25Q_FlashPageProgram(W25Q_TEST_ADDR, tx_data, sizeof(tx_data));
    }

    if (status == W25Q_OK)
    {
        memset(rx_data, 0x00, sizeof(rx_data));

        status = W25Q_FlashRead_Data(W25Q_TEST_ADDR, rx_data, sizeof(tx_data));
    }

    if (status == W25Q_OK)
    {
        if (memcmp(rx_data, tx_data, sizeof(tx_data)) != 0)
        {
            status = W25Q_VERIFY_FAILED;
        }
    }

    /* 5. Page-boundary crossing write test */
    if (status == W25Q_OK)
    {
        status = W25Q_FlashSectorErase(W25Q_TEST_ADDR);
    }

    if (status == W25Q_OK)
    {
        status = W25Q_FlashWrite_NoErase(W25Q_TEST_ADDR + 0xF0U,
                                         page_cross_data,
                                         sizeof(page_cross_data));
    }

    if (status == W25Q_OK)
    {
        status = W25Q_FlashVerify(W25Q_TEST_ADDR + 0xF0U,
                                  page_cross_data,
                                  sizeof(page_cross_data));
    }

    /*
     * 6. Sector-boundary crossing safe update test.
     * This writes 16 bytes in current sector and 16 bytes in next sector.
     */
    if (status == W25Q_OK)
    {
        status = W25Q_FlashUpdate_SectorSafe(W25Q_TEST_ADDR + W25Q_SECTOR_SIZE - 16U,
                                             sector_cross_data,
                                             sizeof(sector_cross_data));
    }

    if (status == W25Q_OK)
    {
        status = W25Q_FlashVerify(W25Q_TEST_ADDR + W25Q_SECTOR_SIZE - 16U,
                                  sector_cross_data,
                                  sizeof(sector_cross_data));
    }

    /*
     * 7. Block erase test
     * This erases 0x10000 bytes from the chip and verifies the deleted bytes
     */

    if (status == W25Q_OK)
    {
    	memset(block_buffer,0xFF,sizeof(block_buffer));
    	status = W25Q_FlashBlockErase_64KB(W25Q_BLOCK_TEST_ADDR);
    	if(status == W25Q_OK)
    	{
    		status = W25Q_FlashVerify(W25Q_BLOCK_TEST_ADDR,block_buffer,sizeof(block_buffer));
    	}
    }

    return status;
}

W25Q_Status_t W25Q_FlashBlockErase_64KB(uint32_t addr)
{
	W25Q_Status_t status = W25Q_OK;
    uint32_t block_base = 0U;

    status = W25Q_CheckRange(addr, 1U);

    if (status == W25Q_OK)
    {
    	block_base = BLOCK64_BASE(addr);

        status = W25Q_FlashWriteEnable();
    }

    if (status == W25Q_OK)
    {

    SPI_DeselectAll();

    W25Q_FLASH_CS_LOW();

    SPI2_TransferByte(W25Q_CMD_BLOCK_ERASE_64KB);

    SPI2_TransferByte((uint8_t)((block_base >> 16U) & 0xFFU));
    SPI2_TransferByte((uint8_t)((block_base >> 8U)  & 0xFFU));
    SPI2_TransferByte((uint8_t)( block_base         & 0xFFU));

    W25Q_FLASH_CS_HIGH();

    status = W25Q_FlashWaitUntilReady();

    }

    return status;
}

W25Q_Status_t W25Q_FlashChipErase_Unsafe(void)
{
	W25Q_Status_t status = W25Q_OK;

	status = W25Q_FlashWriteEnable();

    if (status == W25Q_OK)
    {

		SPI_DeselectAll();

		W25Q_FLASH_CS_LOW();
		SPI2_TransferByte(W25Q_CMD_CHIP_ERASE);
		W25Q_FLASH_CS_HIGH();

		//takes around 42 seconds
		while((W25Q_FlashReadStatusReg1() & W25Q_SR1_BUSY));

    }

    return status;
}
