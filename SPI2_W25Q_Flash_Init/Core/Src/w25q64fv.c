#include "w25q64fv.h"
#include "spi.h"
#include "common.h"

void W25Q_FlashReset(void)
{
    /*
     * Software reset sequence:
     * 1. Enable Reset command: 0x66
     * 2. Reset Device command: 0x99
     *
     * These should be sent as separate SPI instructions.
     */


    W25Q_FLASH_CS_LOW();
    SPI2_TransferByte(W25Q_CMD_ENABLE_RESET);
    W25Q_FLASH_CS_HIGH();

    W25Q_FLASH_CS_LOW();
    SPI2_TransferByte(W25Q_CMD_RESET_DEVICE);
    W25Q_FLASH_CS_HIGH();

    /*
     * Device needs a small time after reset.
     * If you have delay_us(), use around 30 us or more.
     * If not, 1 ms delay is safe for now.
     */
    HAL_Delay(10);
}

void W25Q_FlashReadJEDEC(uint8_t *id)
{
    if (id == 0)
    {
        return;
    }

    W25Q_FLASH_CS_LOW();

    SPI2_TransferByte(W25Q_CMD_READ_JEDEC_ID);

    id[0] = SPI2_TransferByte(W25Q_DUMMY_BYTE);
    id[1] = SPI2_TransferByte(W25Q_DUMMY_BYTE);
    id[2] = SPI2_TransferByte(W25Q_DUMMY_BYTE);

    W25Q_FLASH_CS_HIGH();
}

void W25Q_FlashRead_StatusRegister(uint8_t *status)
{
	//status register 1
	W25Q_FLASH_CS_LOW();
	SPI2_TransferByte(W25Q_CMD_READ_STATUS_REG1);
	status[0] = SPI2_TransferByte(W25Q_DUMMY_BYTE);
	W25Q_FLASH_CS_HIGH();

	//status register 2
	W25Q_FLASH_CS_LOW();
	SPI2_TransferByte(W25Q_CMD_READ_STATUS_REG2);
	status[1] = SPI2_TransferByte(W25Q_DUMMY_BYTE);
	W25Q_FLASH_CS_HIGH();
}

void W25Q_FlashRead_Data(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    if ((buffer == 0) || (size == 0))
    {
        return;
    }

    /* Make sure other SPI2 slave is deselected */
    MCP2515_CS_HIGH();

    W25Q_FLASH_CS_LOW();

    SPI2_TransferByte(W25Q_CMD_READ_DATA);

    /* Send 24-bit address: A23-A16, A15-A8, A7-A0 */
    SPI2_TransferByte((uint8_t)((addr >> 16) & 0xFF));
    SPI2_TransferByte((uint8_t)((addr >> 8)  & 0xFF));
    SPI2_TransferByte((uint8_t)( addr        & 0xFF));

    for (uint32_t i = 0; i < size; i++)
    {
        buffer[i] = SPI2_TransferByte(W25Q_DUMMY_BYTE);
    }

    W25Q_FLASH_CS_HIGH();
}

uint8_t W25Q_FlashReadStatusReg1(void)
{
    uint8_t status;

    MCP2515_CS_HIGH();

    W25Q_FLASH_CS_LOW();
    SPI2_TransferByte(W25Q_CMD_READ_STATUS_REG1);
    status = SPI2_TransferByte(W25Q_DUMMY_BYTE);
    W25Q_FLASH_CS_HIGH();

    return status;
}

void W25Q_FlashWaitUntilReady(void)
{
    while (W25Q_FlashReadStatusReg1() & W25Q_SR1_BUSY)
    {
        /* wait while BUSY = 1 */
    }
}

void W25Q_FlashWriteEnable(void)
{
    W25Q_FLASH_CS_LOW();
    SPI2_TransferByte(W25Q_CMD_WRITE_ENABLE);
    W25Q_FLASH_CS_HIGH();
}

void W25Q_FlashWriteDisable(void)
{
    W25Q_FLASH_CS_LOW();
    SPI2_TransferByte(W25Q_CMD_WRITE_DISABLE);
    W25Q_FLASH_CS_HIGH();
}

void W25Q_FlashSectorErase(uint32_t addr)
{
    W25Q_FlashWriteEnable();

    W25Q_FLASH_CS_LOW();

    SPI2_TransferByte(W25Q_CMD_SECTOR_ERASE_4KB);

    /* Send 24-bit address: A23-A16, A15-A8, A7-A0 */
    SPI2_TransferByte((uint8_t)((addr >> 16) & 0xFF));
    SPI2_TransferByte((uint8_t)((addr >> 8)  & 0xFF));
    SPI2_TransferByte((uint8_t)( addr        & 0xFF));

    W25Q_FLASH_CS_HIGH();

    W25Q_FlashWaitUntilReady();

}

void W25Q_FlashPageProgram(uint32_t addr, uint8_t *data, uint32_t size)
{
    if ((data == 0) || (size == 0))
    {
        return;
    }

    if (size > W25Q_PAGE_SIZE)
    {
        size = W25Q_PAGE_SIZE;
    }

    MCP2515_CS_HIGH();

    W25Q_FlashWriteEnable();

    W25Q_FLASH_CS_LOW();

    SPI2_TransferByte(W25Q_CMD_PAGE_PROGRAM);

    SPI2_TransferByte((uint8_t)((addr >> 16) & 0xFF));
    SPI2_TransferByte((uint8_t)((addr >> 8)  & 0xFF));
    SPI2_TransferByte((uint8_t)( addr        & 0xFF));

    for (uint32_t i = 0; i < size; i++)
    {
        SPI2_TransferByte(data[i]);
    }

    W25Q_FLASH_CS_HIGH();

    W25Q_FlashWaitUntilReady();
}
