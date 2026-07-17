#ifndef INC_W25Q64FV_H_
#define INC_W25Q64FV_H_

#include "stm32f407xx.h"
#include <stdint.h>

/* Memory geometry */
#define W25Q_PAGE_SIZE       0x100UL     /* 256 bytes */
#define W25Q_SECTOR_SIZE     0x1000UL    /* 4 KB */
#define W25Q_BLOCK64_SIZE    0x10000UL   /* 64 KB */
#define W25Q_CHIP_SIZE       0x800000UL  /* 8 MB */

#define PAGE_OFFSET(addr)    ((addr) & 0xFFUL)
#define PAGE_BASE(addr)      ((addr) & ~0xFFUL)

#define SECTOR_OFFSET(addr)  ((addr) & 0xFFFUL)
#define SECTOR_BASE(addr)    ((addr) & ~0xFFFUL)

#define BLOCK64_OFFSET(addr) ((addr) & 0xFFFFUL)
#define BLOCK64_BASE(addr)   ((addr) & ~0xFFFFUL)

/* W25Q64FV command set */
#define W25Q_CMD_READ_STATUS_REG1   0x05
#define W25Q_CMD_READ_STATUS_REG2   0x35
#define W25Q_CMD_READ_DATA          0x03
#define W25Q_CMD_READ_JEDEC_ID      0x9F
#define W25Q_CMD_ENABLE_RESET       0x66
#define W25Q_CMD_RESET_DEVICE       0x99
#define W25Q_CMD_WRITE_ENABLE       0x06
#define W25Q_CMD_WRITE_DISABLE      0x04
#define W25Q_CMD_SECTOR_ERASE_4KB   0x20
#define W25Q_CMD_PAGE_PROGRAM   	0x02

#define W25Q_DUMMY_BYTE             0xFF

#define W25Q_SR1_BUSY    0x01
#define W25Q_SR1_WEL     0x02

void W25Q_FlashReset(void);
void W25Q_FlashReadJEDEC(uint8_t *id);
void W25Q_FlashRead_StatusRegister(uint8_t *status);
void W25Q_FlashRead_Data(uint32_t addr, uint8_t *buffer, uint32_t size);
uint8_t W25Q_FlashReadStatusReg1(void);
void W25Q_FlashWaitUntilReady(void);
void W25Q_FlashWriteEnable(void);
void W25Q_FlashWriteDisable(void);
void W25Q_FlashSectorErase(uint32_t addr);
void W25Q_FlashPageProgram(uint32_t addr, uint8_t *data, uint32_t size);

#endif /* INC_W25Q64FV_H_ */
