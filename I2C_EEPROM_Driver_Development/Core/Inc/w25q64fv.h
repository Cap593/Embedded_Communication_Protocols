#ifndef INC_W25Q64FV_H_
#define INC_W25Q64FV_H_

#include "stm32f407xx.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stddef.h>

/* Memory geometry
 * NOTE:
 * W25Q64FV  = 8 MB  -> 0x800000UL, JEDEC EF 40 17
 * W25Q128xx = 16 MB -> 0x1000000UL, JEDEC EF 40 18
 *
 * Your current safe test area is low address, so either value works for tests.
 * For full-range protection, set this according to actual JEDEC result.
 */
#define W25Q_BLOCK_TEST_ADDR               0x010000UL
#define W25Q_TEST_ADDR               0x001000UL
#define W25Q_WAIT_TIMEOUT_COUNT      1000000UL

#define W25Q_MANUFACTURER_WINBOND    0xEFU
#define W25Q_MEMORY_TYPE_SPI         0x40U
#define W25Q_CAPACITY_64MBIT         0x17U
#define W25Q_CAPACITY_128MBIT        0x18U

#define W25Q_PAGE_SIZE       0x100UL      /* 256 bytes */
#define W25Q_SECTOR_SIZE     0x1000UL     /* 4 KB */
#define W25Q_BLOCK64_SIZE    0x10000UL    /* 64 KB */
#define W25Q_CHIP_SIZE       0x800000UL   /* 8 MB for W25Q64FV */

#define PAGE_OFFSET(addr)    ((addr) & 0xFFUL)
#define PAGE_BASE(addr)      ((addr) & ~0xFFUL)

#define SECTOR_OFFSET(addr)  ((addr) & 0xFFFUL)
#define SECTOR_BASE(addr)    ((addr) & ~0xFFFUL)

#define BLOCK64_OFFSET(addr) ((addr) & 0xFFFFUL)
#define BLOCK64_BASE(addr)   ((addr) & ~0xFFFFUL)

/* W25Q command set */
#define W25Q_CMD_READ_STATUS_REG1   0x05U
#define W25Q_CMD_READ_STATUS_REG2   0x35U
#define W25Q_CMD_READ_DATA          0x03U
#define W25Q_CMD_READ_JEDEC_ID      0x9FU
#define W25Q_CMD_ENABLE_RESET       0x66U
#define W25Q_CMD_RESET_DEVICE       0x99U
#define W25Q_CMD_WRITE_ENABLE       0x06U
#define W25Q_CMD_WRITE_DISABLE      0x04U
#define W25Q_CMD_SECTOR_ERASE_4KB   0x20U
#define W25Q_CMD_PAGE_PROGRAM       0x02U
#define W25Q_CMD_BLOCK_ERASE_64KB   0xD8U
#define W25Q_CMD_CHIP_ERASE		    0xC7U

#define W25Q_DUMMY_BYTE             0xFFU

#define W25Q_SR1_BUSY               0x01U
#define W25Q_SR1_WEL                0x02U

typedef enum
{
    W25Q_OK = 0,
    W25Q_ERROR,
    W25Q_TIMEOUT,
    W25Q_INVALID_PARAM,
    W25Q_VERIFY_FAILED,
    W25Q_UNKNOWN_DEVICE
} W25Q_Status_t;

void W25Q_FlashReset(void);
void W25Q_FlashReadJEDEC(uint8_t *id);
void W25Q_FlashRead_StatusRegister(uint8_t *status);

uint8_t W25Q_FlashReadStatusReg1(void);

W25Q_Status_t W25Q_FlashWaitUntilReady(void);
W25Q_Status_t W25Q_FlashWriteEnable(void);
void W25Q_FlashWriteDisable(void);

W25Q_Status_t W25Q_FlashRead_Data(uint32_t addr, uint8_t *buffer, uint32_t size);
W25Q_Status_t W25Q_FlashSectorErase(uint32_t addr);
W25Q_Status_t W25Q_FlashPageProgram(uint32_t addr, uint8_t *data, uint32_t size);
W25Q_Status_t W25Q_FlashWrite_NoErase(uint32_t addr, uint8_t *data, uint32_t size);
W25Q_Status_t W25Q_FlashUpdate_SectorSafe(uint32_t addr, uint8_t *data, uint32_t size);
W25Q_Status_t W25Q_FlashVerify(uint32_t addr, uint8_t *expected, uint32_t size);
W25Q_Status_t W25Q_FlashSelfTest(void);
W25Q_Status_t W25Q_FlashBlockErase_64KB(uint32_t addr);
W25Q_Status_t W25Q_FlashChipErase_Unsafe(void);

#endif /* INC_W25Q64FV_H_ */
