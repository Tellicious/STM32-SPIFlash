/* BEGIN Header */
/**
 ******************************************************************************
 * @file    SPIFlash.c
 * @author  Andrea Vivani
 * @brief   Generic SPI flash memory driver
 ******************************************************************************
 * @copyright
 *
 * Copyright 2023 Andrea Vivani
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 ******************************************************************************
 */
/* END Header */

/* Includes ------------------------------------------------------------------*/

#include "SPIFlash.h"
#include <string.h>

/* Macros ---------------------------------------------------------------------*/


#define SPIFLASH_PAGE_SIZE                      (1 << 8)
#define SPIFLASH_SECTOR_SIZE                    (1 << 12)
#define SPIFLASH_BLOCK_SIZE                     (1 << 16)

#define SPIFLASH_PAGE2SECTOR(pageNumber)      (pageNumber >> 4) //((pageNumber * SPIFLASH_PAGE_SIZE) / SPIFLASH_SECTOR_SIZE)
#define SPIFLASH_PAGE2BLOCK(pageNumber)       (pageNumber >> 8) //((pageNumber * SPIFLASH_PAGE_SIZE) / SPIFLASH_BLOCK_SIZE)
#define SPIFLASH_SECTOR2BLOCK(sectorNumber)   (sectorNumber >> 4) //((sectorNumber * SPIFLASH_SECTOR_SIZE) / SPIFLASH_BLOCK_SIZE)
#define SPIFLASH_SECTOR2PAGE(sectorNumber)    (sectorNumber << 4) //((sectorNumber * SPIFLASH_SECTOR_SIZE) / SPIFLASH_PAGE_SIZE)
#define SPIFLASH_BLOCK2SECTOR(blockNumber)    (blockNumber << 4)
#define SPIFLASH_BLOCK2PAGE(blockNumber)      (blockNumber << 8) //((blockNumber * SPIFLASH_BLOCK_SIZE) / SPIFLASH_PAGE_SIZE)
#define SPIFLASH_PAGE2ADDRESS(pageNumber)     (pageNumber << 8) //(pageNumber * SPIFLASH_PAGE_SIZE)
#define SPIFLASH_SECTOR2ADDRESS(sectorNumber) (sectorNumber << 12) //(sectorNumber * SPIFLASH_SECTOR_SIZE)
#define SPIFLASH_BLOCK2ADDRESS(blockNumber)   (blockNumber << 16) //(blockNumber * SPIFLASH_BLOCK_SIZE)
#define SPIFLASH_ADDRESS2PAGE(address)        (address >> 8) //(address / SPIFLASH_PAGE_SIZE)
#define SPIFLASH_ADDRESS2SECTOR(address)      (address >> 12) //(address / SPIFLASH_SECTOR_SIZE)
#define SPIFLASH_ADDRESS2BLOCK(address)       (address >> 16) //(address / SPIFLASH_BLOCK_SIZE)

#define SPIFLASH_DUMMY_BYTE 0xA5

#define SPIFLASH_CMD_READSFDP 0x5A
#define SPIFLASH_CMD_ID 0x90
#define SPIFLASH_CMD_JEDECID 0x9F
#define SPIFLASH_CMD_UNIQUEID 0x4B
#define SPIFLASH_CMD_WRITEDISABLE 0x04
#define SPIFLASH_CMD_READSTATUS1 0x05
#define SPIFLASH_CMD_READSTATUS2 0x35
#define SPIFLASH_CMD_READSTATUS3 0x15
#define SPIFLASH_CMD_WRITESTATUSEN 0x50
#define SPIFLASH_CMD_WRITESTATUS1 0x01
#define SPIFLASH_CMD_WRITESTATUS2 0x31
#define SPIFLASH_CMD_WRITESTATUS3 0x11
#define SPIFLASH_CMD_WRITEENABLE 0x06
#define SPIFLASH_CMD_ADDR4BYTE_EN 0xB7
#define SPIFLASH_CMD_ADDR4BYTE_DIS 0xE9
#define SPIFLASH_CMD_PAGEPROG3ADD 0x02
#define SPIFLASH_CMD_PAGEPROG4ADD 0x12
#define SPIFLASH_CMD_READDATA3ADD 0x03
#define SPIFLASH_CMD_READDATA4ADD 0x13
#define SPIFLASH_CMD_FASTREAD3ADD 0x0B
#define SPIFLASH_CMD_FASTREAD4ADD 0x0C
#define SPIFLASH_CMD_SECTORERASE3ADD 0x20
#define SPIFLASH_CMD_SECTORERASE4ADD 0x21
#define SPIFLASH_CMD_BLOCKERASE3ADD 0xD8
#define SPIFLASH_CMD_BLOCKERASE4ADD 0xDC
#define SPIFLASH_CMD_CHIPERASE1 0x60
#define SPIFLASH_CMD_CHIPERASE2 0xC7
#define SPIFLASH_CMD_SUSPEND 0x75
#define SPIFLASH_CMD_RESUME 0x7A
#define SPIFLASH_CMD_POWERDOWN 0xB9
#define SPIFLASH_CMD_RELEASE 0xAB
#define SPIFLASH_CMD_FRAMSERNO 0xC3

#define SPIFlashSTATUS1_BUSY (1 << 0)
#define SPIFlashSTATUS1_WEL (1 << 1)
#define SPIFlashSTATUS1_BP0 (1 << 2)
#define SPIFlashSTATUS1_BP1 (1 << 3)
#define SPIFlashSTATUS1_BP2 (1 << 4)
#define SPIFlashSTATUS1_TP (1 << 5)
#define SPIFlashSTATUS1_SEC (1 << 6)
#define SPIFlashSTATUS1_SRP0 (1 << 7)

#define SPIFlashSTATUS2_SRP1 (1 << 0)
#define SPIFlashSTATUS2_QE (1 << 1)
#define SPIFlashSTATUS2_RESERVE1 (1 << 2)
#define SPIFlashSTATUS2_LB0 (1 << 3)
#define SPIFlashSTATUS2_LB1 (1 << 4)
#define SPIFlashSTATUS2_LB2 (1 << 5)
#define SPIFlashSTATUS2_CMP (1 << 6)
#define SPIFlashSTATUS2_SUS (1 << 7)

#define SPIFlashSTATUS3_RESERVE1 (1 << 0)
#define SPIFlashSTATUS3_RESERVE2 (1 << 1)
#define SPIFlashSTATUS3_WPS (1 << 2)
#define SPIFlashSTATUS3_RESERVE3 (1 << 3)
#define SPIFlashSTATUS3_RESERVE4 (1 << 4)
#define SPIFlashSTATUS3_DRV0 (1 << 5)
#define SPIFlashSTATUS3_DRV1 (1 << 6)
#define SPIFlashSTATUS3_HOLD (1 << 7)

#if SPIFLASH_DEBUG == SPIFLASH_DEBUG_DISABLE
#define dprintf(...)
#else
#include <stdio.h>
extern int _write(int file, char *ptr, int len);
#define dprintf(...) printf(__VA_ARGS__)
#endif

/* HW Interface  functions ----------------------------------------------------*/

#define SPIFlashDelay(x) HAL_Delay(x)

#define SPIFlashGetTick() HAL_GetTick()

static inline void SPIFlashCSPin(SPIFlash_t *SPIFlash, SPIFlashStatus_t Select)
{
	HAL_GPIO_WritePin(SPIFlash->GPIO, SPIFlash->pin, (GPIO_PinState)Select);
}

static SPIFlashStatus_t SPIFlashTransmitReceive(SPIFlash_t *SPIFlash, uint8_t *Tx, uint8_t *Rx, size_t size, uint32_t Timeout)
 {
 #if (SPIFlashPLATFORM == SPIFlashPLATFORM_HAL)
 	if (HAL_SPI_TransmitReceive(SPIFlash->hSPI, Tx, Rx, size, Timeout) == HAL_OK)
 	{
 		return SPIFLASH_SUCCESS;
 	}
 	else
 	{
 		return SPIFLASH_TIMEOUT;
 	}

 #elif (SPIFlashPLATFORM == SPIFlashPLATFORM_HAL_DMA)
 	uint32_t startTime = SPIFlashGetTick();
 	if (HAL_SPI_TransmitReceive_DMA(SPIFlash->hSPI, Tx, Rx, size) != HAL_OK)
 	{
 		return SPIFLASH_ERROR;
 	}
 	else
 	{
 		while (1)
 		{
 			SPIFlashDelay(1);
 			if (SPIFlashGetTick() - startTime >= Timeout)
 			{
 				HAL_SPI_DMAStop(SPIFlash->hSPI);
 				return SPIFLASH_TIMEOUT;
 			}
 			if (HAL_SPI_GetState(SPIFlash->hSPI) == HAL_SPI_STATE_READY)
 			{
 				retVal = SPIFLASH_SUCCESS;
 				return SPIFLASH_SUCCESS;
 			}
 		}
 	}
 #endif
 }


/* Static  functions ----------------------------------------------------------*/

static void SPIFlashLock(SPIFlash_t *SPIFlash)
{
	while (SPIFlash->lock)
	{
		SPIFlashDelay(1);
	}
	SPIFlash->lock = 1;
}

static void SPIFlashUnLock(SPIFlash_t *SPIFlash)
{
	SPIFlash->lock = 0;
}

static SPIFlashStatus_t SPIFlashWriteEnable(SPIFlash_t *SPIFlash)
{
	SPIFlashStatus_t retVal = SPIFLASH_SUCCESS;
	uint8_t tx[1] = {SPIFLASH_CMD_WRITEENABLE};
	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 1, 100) == SPIFLASH_ERROR)
	{
		retVal = SPIFLASH_ERROR;
		dprintf("SPIFlashWriteEnable() Error\r\n");
	}
	SPIFlashCSPin(SPIFlash, 1);
	return retVal;
}

static SPIFlashStatus_t SPIFlashWriteDisable(SPIFlash_t *SPIFlash)
{
	SPIFlashStatus_t retVal = SPIFLASH_SUCCESS;
	uint8_t tx[1] = {SPIFLASH_CMD_WRITEDISABLE};
	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 1, 100) == SPIFLASH_ERROR)
	{
		dprintf("SPIFlashWriteDisable() Error\r\n");
		retVal = SPIFLASH_ERROR;
	}
	SPIFlashCSPin(SPIFlash, 1);
	return retVal;
}

static uint8_t SPIFlashReadReg1(SPIFlash_t *SPIFlash)
{
	uint8_t retVal = 0;
	uint8_t tx[2] = {SPIFLASH_CMD_READSTATUS1, SPIFLASH_DUMMY_BYTE};
	uint8_t rx[2];
	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, tx, rx, 2, 100) == SPIFLASH_SUCCESS)
	{
		retVal = rx[1];
	}
	SPIFlashCSPin(SPIFlash, 1);
	return retVal;
}

static uint8_t SPIFlashReadReg2(SPIFlash_t *SPIFlash)
{
	uint8_t retVal = 0;
	uint8_t tx[2] = {SPIFLASH_CMD_READSTATUS2, SPIFLASH_DUMMY_BYTE};
	uint8_t rx[2];
	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, tx, rx, 2, 100) == SPIFLASH_SUCCESS)
	{
		retVal = rx[1];
	}
	SPIFlashCSPin(SPIFlash, 1);
	return retVal;
}

static uint8_t SPIFlashReadReg3(SPIFlash_t *SPIFlash)
{
	uint8_t retVal = 0;
	uint8_t tx[2] = {SPIFLASH_CMD_READSTATUS3, SPIFLASH_DUMMY_BYTE};
	uint8_t rx[2];
	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, tx, rx, 2, 100) == SPIFLASH_SUCCESS)
	{
		retVal = rx[1];
	}
	SPIFlashCSPin(SPIFlash, 1);
	return retVal;
}

/*
static SPIFlashStatus_t SPIFlashWriteReg1(SPIFlash_t *SPIFlash, uint8_t data)
{
	uint8_t tx[2] = {SPIFLASH_CMD_WRITESTATUS1, data};
	uint8_t cmd = SPIFLASH_CMD_WRITESTATUSEN;

	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, &cmd, &cmd, 1, 100) == SPIFLASH_ERROR)
	{
		SPIFlashCSPin(SPIFlash, 1);
		return SPIFLASH_ERROR;
	}
	SPIFlashCSPin(SPIFlash, 1);
	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 2, 100) == SPIFLASH_ERROR)
	{
		SPIFlashCSPin(SPIFlash, 1);
		return SPIFLASH_ERROR;
	}
	SPIFlashCSPin(SPIFlash, 1);

	return SPIFLASH_SUCCESS;
}

static SPIFlashStatus_t SPIFlashWriteReg2(SPIFlash_t *SPIFlash, uint8_t data)
{
	uint8_t tx[2] = {SPIFLASH_CMD_WRITESTATUS2, data};
	uint8_t cmd = SPIFLASH_CMD_WRITESTATUSEN;

	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, &cmd, &cmd, 1, 100) == SPIFLASH_ERROR)
	{
		SPIFlashCSPin(SPIFlash, 1);
		return SPIFLASH_ERROR;
	}
	SPIFlashCSPin(SPIFlash, 1);
	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 2, 100) == SPIFLASH_ERROR)
	{
		SPIFlashCSPin(SPIFlash, 1);
		return SPIFLASH_ERROR;
	}
	SPIFlashCSPin(SPIFlash, 1);

	return SPIFLASH_SUCCESS;
}

static SPIFlashStatus_t SPIFlashWriteReg3(SPIFlash_t *SPIFlash, uint8_t data)
{
	uint8_t tx[2] = {SPIFLASH_CMD_WRITESTATUS3, data};
	uint8_t cmd = SPIFLASH_CMD_WRITESTATUSEN;

	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, &cmd, &cmd, 1, 100) == SPIFLASH_ERROR)
	{
		SPIFlashCSPin(SPIFlash, 1);
		return SPIFLASH_ERROR;
	}
	SPIFlashCSPin(SPIFlash, 1);
	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 2, 100) == SPIFLASH_ERROR)
	{
		SPIFlashCSPin(SPIFlash, 1);
		return SPIFLASH_ERROR;
	}
	SPIFlashCSPin(SPIFlash, 1);

	return SPIFLASH_SUCCESS;
}
*/

static SPIFlashStatus_t SPIFlashWaitForWriting(SPIFlash_t *SPIFlash, uint32_t Timeout)
{
	uint32_t startTime = SPIFlashGetTick();
	while (1)
	{
		if (SPIFlashGetTick() - startTime >= Timeout)
		{
			return SPIFLASH_TIMEOUT;
		}
		if ((SPIFlashReadReg1(SPIFlash) & SPIFlashSTATUS1_BUSY) == 0)
		{
			return SPIFLASH_SUCCESS;
		}
		SPIFlashDelay(1);
	}
}

static SPIFlashStatus_t SPIFlashFindChip(SPIFlash_t *SPIFlash)
{
	uint8_t tx[4] = {SPIFLASH_CMD_JEDECID, 0xFF, 0xFF, 0xFF};
	uint8_t rx[4];

	SPIFlashCSPin(SPIFlash, 0);
	if (SPIFlashTransmitReceive(SPIFlash, tx, rx, 4, 100) == SPIFLASH_ERROR)
	{
		SPIFlashCSPin(SPIFlash, 1);
		return SPIFLASH_ERROR;
	}
	SPIFlashCSPin(SPIFlash, 1);
	dprintf("CHIP ID: 0x%02X%02X%02X\r\n", rx[1], rx[2], rx[3]);
	SPIFlash->manufacturer = rx[1];
	SPIFlash->memType = rx[2];
	SPIFlash->size = rx[3];

	dprintf("SPI FLASH MANUFACTURER: ");
	switch (SPIFlash->manufacturer)
	{
	case SPIFLASH_MANUFACTURER_WINBOND:
		dprintf("WINBOND");
		break;
	case SPIFLASH_MANUFACTURER_SPANSION:
		dprintf("SPANSION");
		break;
	case SPIFLASH_MANUFACTURER_MICRON:
		dprintf("MICRON");
		break;
	case SPIFLASH_MANUFACTURER_MACRONIX:
		dprintf("MACRONIX");
		break;
	case SPIFLASH_MANUFACTURER_ISSI:
		dprintf("ISSI");
		break;
	case SPIFLASH_MANUFACTURER_GIGADEVICE:
		dprintf("GIGADEVICE");
		break;
	case SPIFLASH_MANUFACTURER_AMIC:
		dprintf("AMIC");
		break;
	case SPIFLASH_MANUFACTURER_SST:
		dprintf("SST");
		break;
	case SPIFLASH_MANUFACTURER_HYUNDAI:
		dprintf("HYUNDAI");
		break;
	case SPIFLASH_MANUFACTURER_FUDAN:
		dprintf("FUDAN");
		break;
	case SPIFLASH_MANUFACTURER_ESMT:
		dprintf("ESMT");
		break;
	case SPIFLASH_MANUFACTURER_INTEL:
		dprintf("INTEL");
		break;
	case SPIFLASH_MANUFACTURER_SANYO:
		dprintf("SANYO");
		break;
	case SPIFLASH_MANUFACTURER_FUJITSU:
		dprintf("FUJITSU");
		break;
	case SPIFLASH_MANUFACTURER_EON:
		dprintf("EON");
		break;
	case SPIFLASH_MANUFACTURER_PUYA:
		dprintf("PUYA");
		break;
	default:
		SPIFlash->manufacturer = SPIFLASH_MANUFACTURER_ERROR;
		dprintf("ERROR");
		break;
	}
	dprintf(" - MEMTYPE: 0x%02X", SPIFlash->memType);
	dprintf(" - SIZE: ");

	switch (SPIFlash->size)
	{
	case SPIFLASH_SIZE_1MBIT:
		SPIFlash->blockNum = 2;
		dprintf("1 MBIT\r\n");
		break;
	case SPIFLASH_SIZE_2MBIT:
		SPIFlash->blockNum = 4;
		dprintf("2 MBIT\r\n");
		break;
	case SPIFLASH_SIZE_4MBIT:
		SPIFlash->blockNum = 8;
		dprintf("4 MBIT\r\n");
		break;
	case SPIFLASH_SIZE_8MBIT:
		SPIFlash->blockNum = 16;
		dprintf("8 MBIT\r\n");
		break;
	case SPIFLASH_SIZE_16MBIT:
		SPIFlash->blockNum = 32;
		dprintf("16 MBIT\r\n");
		break;
	case SPIFLASH_SIZE_32MBIT:
		SPIFlash->blockNum = 64;
		dprintf("32 MBIT\r\n");
		break;
	case SPIFLASH_SIZE_64MBIT:
		SPIFlash->blockNum = 128;
		dprintf("64 MBIT\r\n");
		break;
	case SPIFLASH_SIZE_128MBIT:
		SPIFlash->blockNum = 256;
		dprintf("128 MBIT\r\n");
		break;
	case SPIFLASH_SIZE_256MBIT:
		SPIFlash->blockNum = 512;
		dprintf("256 MBIT\r\n");
		break;
	case SPIFLASH_SIZE_512MBIT:
		SPIFlash->blockNum = 1024;
		dprintf("512 MBIT\r\n");
		break;
	default:
		SPIFlash->size = SPIFLASH_SIZE_ERROR;
		dprintf("ERROR\r\n");
		break;
	}

	SPIFlash->sectorNum = SPIFLASH_BLOCK2SECTOR(SPIFlash->blockNum);
	SPIFlash->pageNum = SPIFLASH_SECTOR2PAGE(SPIFlash->sectorNum);
	dprintf("SPI FLASH BLOCK CNT: %ld\r\n", SPIFlash->blockNum);
	dprintf("SPI FLASH SECTOR CNT: %ld\r\n", SPIFlash->sectorNum);
	dprintf("SPI FLASH PAGE CNT: %ld\r\n", SPIFlash->pageNum);
	dprintf("SPI FLASH STATUS1: 0x%02X\r\n", SPIFlashReadReg1(SPIFlash));
	dprintf("SPI FLASH STATUS2: 0x%02X\r\n", SPIFlashReadReg2(SPIFlash));
	dprintf("SPI FLASH STATUS3: 0x%02X\r\n", SPIFlashReadReg3(SPIFlash));
	return SPIFLASH_SUCCESS;
}

static SPIFlashStatus_t SPIFlashWriteFn(SPIFlash_t *SPIFlash, uint32_t pageNumber, uint8_t *data, uint32_t size, uint32_t offset)
{
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	uint32_t address = 0, maximum = SPIFLASH_PAGE_SIZE - offset;
	uint8_t tx[5];
	do
	{
#if SPIFLASH_DEBUG != SPIFLASH_DEBUG_DISABLE
		uint32_t dbgTime = SPIFlashGetTick();
#endif
		dprintf("SPIFlashWritePage() START PAGE %ld\r\n", pageNumber);
		if (pageNumber >= SPIFlash->pageNum)
		{
			break;
		}
		if (offset >= SPIFLASH_PAGE_SIZE)
		{
			break;
		}
		if (size > maximum)
		{
			size = maximum;
		}
		address = SPIFLASH_PAGE2ADDRESS(pageNumber) + offset;

#if SPIFLASH_DEBUG == SPIFLASH_DEBUG_FULL
			dprintf("SPI FLASH WRITING {\r\n0x%02X", data[0]);
			for (int i = 1; i < size; i++)
			{
				if (i % 8 == 0)
				{
					dprintf("\r\n");
				}
				dprintf(", 0x%02X", data[i]);
			}
			dprintf("\r\n}\r\n");
#endif

		if (SPIFlashWriteEnable(SPIFlash) == SPIFLASH_ERROR)
		{
			break;
		}

		SPIFlashCSPin(SPIFlash, 0);
		if (SPIFlash->blockNum >= 512)
		{
			tx[0] = SPIFLASH_CMD_PAGEPROG4ADD;
			tx[1] = (address & 0xFF000000) >> 24;
			tx[2] = (address & 0x00FF0000) >> 16;
			tx[3] = (address & 0x0000FF00) >> 8;
			tx[4] = (address & 0x000000FF);
			if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 5, 100) == SPIFLASH_ERROR)
			{
				SPIFlashCSPin(SPIFlash, 1);
				break;
			}
		}
		else
		{
			tx[0] = SPIFLASH_CMD_PAGEPROG3ADD;
			tx[1] = (address & 0x00FF0000) >> 16;
			tx[2] = (address & 0x0000FF00) >> 8;
			tx[3] = (address & 0x000000FF);
			if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 4, 100) == SPIFLASH_ERROR)
			{
				SPIFlashCSPin(SPIFlash, 1);
				break;
			}
		}
		if (SPIFlashTransmitReceive(SPIFlash, data, data, size, 1000) == SPIFLASH_ERROR)
		{
			SPIFlashCSPin(SPIFlash, 1);
			break;
		}
		SPIFlashCSPin(SPIFlash, 1);
		if (SPIFlashWaitForWriting(SPIFlash, 100))
		{
			dprintf("SPIFlashWritePage() %d BYTES WRITTEN IN %ld ms\r\n", (uint16_t)size, SPIFlashGetTick() - dbgTime);
			retVal = SPIFLASH_SUCCESS;
		}

	} while (0);

	SPIFlashWriteDisable(SPIFlash);
	return retVal;
}

static SPIFlashStatus_t SPIFlashReadFn(SPIFlash_t *SPIFlash, uint32_t address, uint8_t *data, uint32_t size)
{
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	uint8_t tx[5];
	do
	{

#if SPIFLASH_DEBUG != SPIFLASH_DEBUG_DISABLE
		uint32_t dbgTime = SPIFlashGetTick();
#endif
		dprintf("SPIFlashReadAddress() START ADDRESS %ld\r\n", address);
		SPIFlashCSPin(SPIFlash, 0);
		if (SPIFlash->blockNum >= 512)
		{
			tx[0] = SPIFLASH_CMD_READDATA4ADD;
			tx[1] = (address & 0xFF000000) >> 24;
			tx[2] = (address & 0x00FF0000) >> 16;
			tx[3] = (address & 0x0000FF00) >> 8;
			tx[4] = (address & 0x000000FF);
			if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 5, 100) == SPIFLASH_ERROR)
			{
				SPIFlashCSPin(SPIFlash, 1);
				break;
			}
		}
		else
		{
			tx[0] = SPIFLASH_CMD_READDATA3ADD;
			tx[1] = (address & 0x00FF0000) >> 16;
			tx[2] = (address & 0x0000FF00) >> 8;
			tx[3] = (address & 0x000000FF);
			if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 4, 100) == SPIFLASH_ERROR)
			{
				SPIFlashCSPin(SPIFlash, 1);
				break;
			}
		}
		if (SPIFlashTransmitReceive(SPIFlash, data, data, size, 2000) == SPIFLASH_ERROR)
		{
			SPIFlashCSPin(SPIFlash, 1);
			break;
		}
		SPIFlashCSPin(SPIFlash, 1);
		dprintf("SPIFlashReadAddress() %d BYTES READ IN %ld ms\r\n", (uint16_t)size, SPIFlashGetTick() - dbgTime);

#if SPIFLASH_DEBUG == SPIFLASH_DEBUG_FULL
		dprintf("{\r\n0x%02X", data[0]);
		for (int i = 1; i < size; i++)
		{
			if (i % 8 == 0)
			{
				dprintf("\r\n");
			}
			dprintf(", 0x%02X", data[i]);
		}
		dprintf("\r\n}\r\n");
#endif

		retVal = SPIFLASH_SUCCESS;

	} while (0);

	return retVal;
}

/* Private  functions ---------------------------------------------------------*/

SPIFlashStatus_t SPIFlashInit(SPIFlash_t *SPIFlash, SPI_HandleTypeDef *hSPI, GPIO_TypeDef *GPIO, uint16_t pin)
{

	if ((SPIFlash == NULL) || (hSPI == NULL) || (GPIO == NULL) || (SPIFlash->size != SPIFLASH_SIZE_ERROR))
	{
		return SPIFLASH_ERROR;
	}

	memset(SPIFlash, 0, sizeof(SPIFlash_t));
	SPIFlash->hSPI = hSPI;
	SPIFlash->GPIO = GPIO;
	SPIFlash->pin = pin;
	SPIFlash->size = SPIFLASH_SIZE_ERROR;
	SPIFlashCSPin(SPIFlash, 1);

	/* Wait for stable VCC */
	while (SPIFlashGetTick() < 20)
	{
		SPIFlashDelay(1);
	}

	if (SPIFlashWriteDisable(SPIFlash) == SPIFLASH_ERROR)
	{
		return SPIFLASH_ERROR;
	}

	if (SPIFlashFindChip(SPIFlash) == SPIFLASH_ERROR)
	{
		return SPIFLASH_ERROR;
	}

	return SPIFLASH_SUCCESS;
}

SPIFlashStatus_t SPIFlashEraseChip(SPIFlash_t *SPIFlash)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	uint8_t tx[1] = {SPIFLASH_CMD_CHIPERASE1};
	do
	{
#if SPIFLASH_DEBUG != SPIFLASH_DEBUG_DISABLE
		uint32_t dbgTime = SPIFlashGetTick();
#endif
		dprintf("SPIFlashEraseChip() START\r\n");
		if (SPIFlashWriteEnable(SPIFlash) == SPIFLASH_ERROR)
		{
			break;
		}
		SPIFlashCSPin(SPIFlash, 0);
		if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 1, 100) == SPIFLASH_ERROR)
		{
			SPIFlashCSPin(SPIFlash, 1);
			break;
		}
		SPIFlashCSPin(SPIFlash, 1);
		if (SPIFlashWaitForWriting(SPIFlash, SPIFlash->blockNum * 1000))
		{
			dprintf("SPIFlashEraseChip() DONE IN %ld ms\r\n", SPIFlashGetTick() - dbgTime);
			retVal = SPIFLASH_SUCCESS;
		}

	} while (0);

	SPIFlashWriteDisable(SPIFlash);
	SPIFlashUnLock(SPIFlash);
	return retVal;
}

SPIFlashStatus_t SPIFlashEraseSector(SPIFlash_t *SPIFlash, uint32_t sector)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	uint32_t address = sector * SPIFLASH_SECTOR_SIZE;
	uint8_t tx[5];
	do
	{
#if SPIFLASH_DEBUG != SPIFLASH_DEBUG_DISABLE
		uint32_t dbgTime = SPIFlashGetTick();
#endif
		dprintf("SPIFlashEraseSector() START SECTOR %ld\r\n", sector);
		if (sector >= SPIFlash->sectorNum)
		{
			dprintf("SPIFlashEraseSector() ERROR SECTOR NUMBER\r\n");
			break;
		}
		if (SPIFlashWriteEnable(SPIFlash) == SPIFLASH_ERROR)
		{
			break;
		}
		SPIFlashCSPin(SPIFlash, 0);
		if (SPIFlash->blockNum >= 512)
		{
			tx[0] = SPIFLASH_CMD_SECTORERASE4ADD;
			tx[1] = (address & 0xFF000000) >> 24;
			tx[2] = (address & 0x00FF0000) >> 16;
			tx[3] = (address & 0x0000FF00) >> 8;
			tx[4] = (address & 0x000000FF);
			if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 5, 100) == SPIFLASH_ERROR)
			{
				SPIFlashCSPin(SPIFlash, 1);
				break;
			}
		}
		else
		{
			tx[0] = SPIFLASH_CMD_SECTORERASE3ADD;
			tx[1] = (address & 0x00FF0000) >> 16;
			tx[2] = (address & 0x0000FF00) >> 8;
			tx[3] = (address & 0x000000FF);
			if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 4, 100) == SPIFLASH_ERROR)
			{
				SPIFlashCSPin(SPIFlash, 1);
				break;
			}
		}
		SPIFlashCSPin(SPIFlash, 1);
		if (SPIFlashWaitForWriting(SPIFlash, 1000))
		{
			dprintf("SPIFlashEraseSector() DONE AFTER %ld ms\r\n", SPIFlashGetTick() - dbgTime);
			retVal = SPIFLASH_SUCCESS;
		}

	} while (0);

	SPIFlashWriteDisable(SPIFlash);
	SPIFlashUnLock(SPIFlash);
	return retVal;
}

SPIFlashStatus_t SPIFlashEraseBlock(SPIFlash_t *SPIFlash, uint32_t block)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	uint32_t address = block * SPIFLASH_BLOCK_SIZE;
	uint8_t tx[5];
	do
	{
#if SPIFLASH_DEBUG != SPIFLASH_DEBUG_DISABLE
		uint32_t dbgTime = SPIFlashGetTick();
#endif
		dprintf("SPIFlashEraseBlock() START PAGE %ld\r\n", block);
		if (block >= SPIFlash->blockNum)
		{
			dprintf("SPIFlashEraseBlock() ERROR BLOCK NUMBER\r\n");
			break;
		}
		if (SPIFlashWriteEnable(SPIFlash) == SPIFLASH_ERROR)
		{
			break;
		}
		SPIFlashCSPin(SPIFlash, 0);
		if (SPIFlash->blockNum >= 512)
		{
			tx[0] = SPIFLASH_CMD_BLOCKERASE4ADD;
			tx[1] = (address & 0xFF000000) >> 24;
			tx[2] = (address & 0x00FF0000) >> 16;
			tx[3] = (address & 0x0000FF00) >> 8;
			tx[4] = (address & 0x000000FF);
			if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 5, 100) == SPIFLASH_ERROR)
			{
				SPIFlashCSPin(SPIFlash, 1);
				break;
			}
		}
		else
		{
			tx[0] = SPIFLASH_CMD_BLOCKERASE3ADD;
			tx[1] = (address & 0x00FF0000) >> 16;
			tx[2] = (address & 0x0000FF00) >> 8;
			tx[3] = (address & 0x000000FF);
			if (SPIFlashTransmitReceive(SPIFlash, tx, tx, 4, 100) == SPIFLASH_ERROR)
			{
				SPIFlashCSPin(SPIFlash, 1);
				break;
			}
		}
		SPIFlashCSPin(SPIFlash, 1);
		if (SPIFlashWaitForWriting(SPIFlash, 3000))
		{
			dprintf("SPIFlashEraseBlock() DONE AFTER %ld ms\r\n", SPIFlashGetTick() - dbgTime);
			retVal = SPIFLASH_SUCCESS;
		}

	} while (0);

	SPIFlashWriteDisable(SPIFlash);
	SPIFlashUnLock(SPIFlash);
	return retVal;
}

SPIFlashStatus_t SPIFlashWriteAddress(SPIFlash_t *SPIFlash, uint32_t address, uint8_t *data, uint32_t size)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	uint32_t page, add, offset, remaining, length, index = 0;
	add = address;
	remaining = size;
	do
	{
		page = SPIFLASH_ADDRESS2PAGE(add);
		offset = add % SPIFLASH_PAGE_SIZE;
		if (remaining <= SPIFLASH_PAGE_SIZE)
		{
			length = remaining;
		}
		else
		{
			length = SPIFLASH_PAGE_SIZE - offset;
		}
		if (SPIFlashWriteFn(SPIFlash, page, &data[index], length, offset) == SPIFLASH_ERROR)
		{
			break;
		}
		add += length;
		index += length;
		remaining -= length;
		if (remaining == 0)
		{
			retVal = SPIFLASH_SUCCESS;
			break;
		}

	} while (remaining > 0);

	SPIFlashUnLock(SPIFlash);
	return retVal;
}

SPIFlashStatus_t SPIFlashWritePage(SPIFlash_t *SPIFlash, uint32_t pageNumber, uint8_t *data, uint32_t size, uint32_t offset)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	retVal = SPIFlashWriteFn(SPIFlash, pageNumber, data, size, offset);
	SPIFlashUnLock(SPIFlash);
	return retVal;
}

SPIFlashStatus_t SPIFlashWriteSector(SPIFlash_t *SPIFlash, uint32_t sectorNumber, uint8_t *data, uint32_t size, uint32_t offset)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_SUCCESS;
	do
	{
		if (offset >= SPIFLASH_SECTOR_SIZE)
		{
			retVal = SPIFLASH_ERROR;
			break;
		}
		if (size > (SPIFLASH_SECTOR_SIZE - offset))
		{
			size = SPIFLASH_SECTOR_SIZE - offset;
		}
		uint32_t bytesWritten = 0;
		uint32_t pageNumber = sectorNumber * (SPIFLASH_SECTOR_SIZE / SPIFLASH_PAGE_SIZE);
		pageNumber += offset / SPIFLASH_PAGE_SIZE;
		uint32_t remainingBytes = size;
		uint32_t pageOffset = offset % SPIFLASH_PAGE_SIZE;
		while (remainingBytes > 0 && pageNumber < ((sectorNumber + 1) * (SPIFLASH_SECTOR_SIZE / SPIFLASH_PAGE_SIZE)))
		{
			uint32_t bytesToWrite = (remainingBytes > SPIFLASH_PAGE_SIZE) ? SPIFLASH_PAGE_SIZE : remainingBytes;
			if (SPIFlashWriteFn(SPIFlash, pageNumber, data + bytesWritten, bytesToWrite, pageOffset) == SPIFLASH_ERROR)
			{
				retVal = SPIFLASH_ERROR;
				break;
			}
			bytesWritten += bytesToWrite;
			remainingBytes -= bytesToWrite;
			pageNumber++;
			pageOffset = 0;
		}
	} while (0);
	SPIFlashUnLock(SPIFlash);
	return retVal;
}

SPIFlashStatus_t SPIFlashWriteBlock(SPIFlash_t *SPIFlash, uint32_t blockNumber, uint8_t *data, uint32_t size, uint32_t offset)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_SUCCESS;
	do
	{
		if (offset >= SPIFLASH_BLOCK_SIZE)
		{
			retVal = SPIFLASH_ERROR;
			break;
		}
		if (size > (SPIFLASH_BLOCK_SIZE - offset))
		{
			size = SPIFLASH_BLOCK_SIZE - offset;
		}
		uint32_t bytesWritten = 0;
		uint32_t pageNumber = blockNumber * (SPIFLASH_BLOCK_SIZE / SPIFLASH_PAGE_SIZE);
		pageNumber += offset / SPIFLASH_PAGE_SIZE;
		uint32_t remainingBytes = size;
		uint32_t pageOffset = offset % SPIFLASH_PAGE_SIZE;
		while (remainingBytes > 0 && pageNumber < ((blockNumber + 1) * (SPIFLASH_BLOCK_SIZE / SPIFLASH_PAGE_SIZE)))
		{
			uint32_t bytesToWrite = (remainingBytes > SPIFLASH_PAGE_SIZE) ? SPIFLASH_PAGE_SIZE : remainingBytes;
			if (SPIFlashWriteFn(SPIFlash, pageNumber, data + bytesWritten, bytesToWrite, pageOffset) == SPIFLASH_ERROR)
			{
				retVal = SPIFLASH_ERROR;
				break;
			}
			bytesWritten += bytesToWrite;
			remainingBytes -= bytesToWrite;
			pageNumber++;
			pageOffset = 0;
		}

	} while (0);

	SPIFlashUnLock(SPIFlash);
	return retVal;
}

SPIFlashStatus_t SPIFlashReadAddress(SPIFlash_t *SPIFlash, uint32_t address, uint8_t *data, uint32_t size)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	retVal = SPIFlashReadFn(SPIFlash, address, data, size);
	SPIFlashUnLock(SPIFlash);
	return retVal;
}

SPIFlashStatus_t SPIFlashReadPage(SPIFlash_t *SPIFlash, uint32_t pageNumber, uint8_t *data, uint32_t size, uint32_t offset)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	uint32_t address = SPIFLASH_PAGE2ADDRESS(pageNumber);
	uint32_t maximum = SPIFLASH_PAGE_SIZE - offset;
	if (size > maximum)
	{
		size = maximum;
	}
	retVal = SPIFlashReadFn(SPIFlash, address, data, size);
	SPIFlashUnLock(SPIFlash);
	return retVal;
}

SPIFlashStatus_t SPIFlashReadSector(SPIFlash_t *SPIFlash, uint32_t sectorNumber, uint8_t *data, uint32_t size, uint32_t offset)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	uint32_t address = SPIFLASH_SECTOR2ADDRESS(sectorNumber);
	uint32_t maximum = SPIFLASH_SECTOR_SIZE - offset;
	if (size > maximum)
	{
		size = maximum;
	}
	retVal = SPIFlashReadFn(SPIFlash, address, data, size);
	SPIFlashUnLock(SPIFlash);
	return retVal;
}

SPIFlashStatus_t SPIFlashReadBlock(SPIFlash_t *SPIFlash, uint32_t blockNumber, uint8_t *data, uint32_t size, uint32_t offset)
{
	SPIFlashLock(SPIFlash);
	SPIFlashStatus_t retVal = SPIFLASH_ERROR;
	uint32_t address = SPIFLASH_BLOCK2ADDRESS(blockNumber);
	uint32_t maximum = SPIFLASH_BLOCK_SIZE - offset;
	if (size > maximum)
	{
		size = maximum;
	}
	retVal = SPIFlashReadFn(SPIFlash, address, data, size);
	SPIFlashUnLock(SPIFlash);
	return retVal;
}
