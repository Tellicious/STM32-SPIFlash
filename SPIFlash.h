/* BEGIN Header */
/**
 ******************************************************************************
 * @file    SPIFlash.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "spi.h"

/* Macros --------------------------------------------------------------------*/

#define SPIFLASH_DEBUG_DISABLE                    0
#define SPIFLASH_DEBUG_MIN                        1
#define SPIFLASH_DEBUG_FULL                       2

#define SPIFLASH_PLATFORM_HAL                     0
#define SPIFLASH_PLATFORM_HAL_DMA                 1

/*---------- SPIFLASH_DEBUG  -----------*/
#define SPIFLASH_DEBUG      SPIFLASH_DEBUG_FULL

/*---------- SPIFLASH_PLATFORM  -----------*/
#define SPIFLASH_PLATFORM      SPIFLASH_PLATFORM_HAL

/* Typedefs ------------------------------------------------------------------*/

/*!
 * SPI flash return status
 */
typedef enum
{
    SPIFLASH_SUCCESS = 0,
    SPIFLASH_ERROR = 1,
	SPIFLASH_TIMEOUT = 2
} SPIFlashStatus_t;

/*!
 * SPI flash manufacturer
 */
typedef enum
{
	SPIFLASH_MANUFACTURER_ERROR = 0,
	SPIFLASH_MANUFACTURER_WINBOND = 0xEF,
	SPIFLASH_MANUFACTURER_ISSI = 0xD5,
	SPIFLASH_MANUFACTURER_MICRON = 0x20,
	SPIFLASH_MANUFACTURER_GIGADEVICE = 0xC8,
	SPIFLASH_MANUFACTURER_MACRONIX = 0xC2,
	SPIFLASH_MANUFACTURER_SPANSION = 0x01,
	SPIFLASH_MANUFACTURER_AMIC = 0x37,
	SPIFLASH_MANUFACTURER_SST = 0xBF,
	SPIFLASH_MANUFACTURER_HYUNDAI = 0xAD,
	SPIFLASH_MANUFACTURER_ATMEL = 0x1F,
	SPIFLASH_MANUFACTURER_FUDAN = 0xA1,
	SPIFLASH_MANUFACTURER_ESMT = 0x8C,
	SPIFLASH_MANUFACTURER_INTEL = 0x89,
	SPIFLASH_MANUFACTURER_SANYO = 0x62,
	SPIFLASH_MANUFACTURER_FUJITSU = 0x04,
	SPIFLASH_MANUFACTURER_EON = 0x1C,
	SPIFLASH_MANUFACTURER_PUYA = 0x85,
} SPIFlashManufacturer_t;

/*!
 * SPI flash size
 */
typedef enum
{
	SPIFLASH_SIZE_ERROR = 0,
	SPIFLASH_SIZE_1MBIT = 0x11,
	SPIFLASH_SIZE_2MBIT = 0x12,
	SPIFLASH_SIZE_4MBIT = 0x13,
	SPIFLASH_SIZE_8MBIT = 0x14,
	SPIFLASH_SIZE_16MBIT = 0x15,
	SPIFLASH_SIZE_32MBIT = 0x16,
	SPIFLASH_SIZE_64MBIT = 0x17,
	SPIFLASH_SIZE_128MBIT = 0x18,
	SPIFLASH_SIZE_256MBIT = 0x19,
	SPIFLASH_SIZE_512MBIT = 0x20,
} SPIFlashSize_t;

/*!
 * SPI flash struct
 */
typedef struct
{
	SPI_HandleTypeDef      *hSPI;
	GPIO_TypeDef           *GPIO;
	SPIFlashManufacturer_t manufacturer;
	SPIFlashSize_t	       size;
	uint8_t                memType, lock;
	uint32_t               pin, pageNum, sectorNum, blockNum;
} SPIFlash_t;

/* Function prototypes --------------------------------------------------------*/

/*!
 * @brief Init SPI flash memory structure
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] hSPI    		pointer to SPI interface handle
 * @param[in] GPIO   		Chip-Select pin GPIO port
 * @param[in] pin   		Chip-Select pin number
 *
 * @return SPIFLASH_SUCCESS if memory data can be read correctly and memory is initialized, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashInit(SPIFlash_t *SPIFlash, SPI_HandleTypeDef *hSPI, GPIO_TypeDef *GPIO, uint16_t pin);

/*!
 * @brief Erase entire SPI flash memory
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 *
 * @return SPIFLASH_SUCCESS if chip is erased successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashEraseChip(SPIFlash_t *SPIFlash);

/*!
 * @brief Erase SPI flash memory sector
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] sector		number of sector to be erased
 *
 * @return SPIFLASH_SUCCESS if sector is erased successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashEraseSector(SPIFlash_t *SPIFlash, uint32_t sector);

/*!
 * @brief Erase SPI flash memory block
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] block			number of block to be erased
 *
 * @return SPIFLASH_SUCCESS if block is erased successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashEraseBlock(SPIFlash_t *SPIFlash, uint32_t block);

/*!
 * @brief Write at a specific address of SPI flash memory
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] address		address of first byte to be written
 * @param[in] data			pointer to data to be written
 * @param[in] size			number of bytes to be written
 *
 * @return SPIFLASH_SUCCESS if data is written successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashWriteAddress(SPIFlash_t *SPIFlash, uint32_t address, uint8_t *data, uint32_t size);

/*!
 * @brief Write at a specific page of SPI flash memory
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] pageNumber		number of the page where to write the data
 * @param[in] data			pointer to data to be written
 * @param[in] size			number of bytes to be written
 * @param[in] offset		offset from beginning of page of first byte to be written
 *
 * @return SPIFLASH_SUCCESS if data is written successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashWritePage(SPIFlash_t *SPIFlash, uint32_t pageNumber, uint8_t *data, uint32_t size, uint32_t offset);

/*!
 * @brief Write at a specific sector of SPI flash memory
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] sectorNumber	number of the sector where to write the data
 * @param[in] data			pointer to data to be written
 * @param[in] size			number of bytes to be written
 * @param[in] offset		offset from beginning of sector of first byte to be written
 *
 * @return SPIFLASH_SUCCESS if data is written successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashWriteSector(SPIFlash_t *SPIFlash, uint32_t sectorNumber, uint8_t *data, uint32_t size, uint32_t offset);

/*!
 * @brief Write at a specific block of SPI flash memory
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] blockNumber	number of the block where to write the data
 * @param[in] data			pointer to data to be written
 * @param[in] size			number of bytes to be written
 * @param[in] offset		offset from beginning of block of first byte to be written
 *
 * @return SPIFLASH_SUCCESS if data is written successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashWriteBlock(SPIFlash_t *SPIFlash, uint32_t blockNumber, uint8_t *data, uint32_t size, uint32_t offset);

/*!
 * @brief Read from a specific address of SPI flash memory
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] address		address of first byte to be read
 * @param[out] data			pointer to data to be read
 * @param[in] size			number of bytes to be read
 *
 * @return SPIFLASH_SUCCESS if data is written successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashReadAddress(SPIFlash_t *SPIFlash, uint32_t address, uint8_t *data, uint32_t size);

/*!
 * @brief Read from a specific page of SPI flash memory
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] pageNumber		number of the page where to read the data
 * @param[out] data			pointer to data to be read
 * @param[in] size			number of bytes to be read
 * @param[in] offset		offset from beginning of page of first byte to be read
 *
 * @return SPIFLASH_SUCCESS if data is read successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashReadPage(SPIFlash_t *SPIFlash, uint32_t pageNumber, uint8_t *data, uint32_t size, uint32_t offset);

/*!
 * @brief Read from a specific sector of SPI flash memory
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] sectorNumber	number of the sector where to read the data
 * @param[out] data			pointer to data to be read
 * @param[in] size			number of bytes to be read
 * @param[in] offset		offset from beginning of sector of first byte to be read
 *
 * @return SPIFLASH_SUCCESS if data is read successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashReadSector(SPIFlash_t *SPIFlash, uint32_t sectorNumber, uint8_t *data, uint32_t size, uint32_t offset);

/*!
 * @brief Read from a specific block of SPI flash memory
 *
 * @param[in] SPIFlash      pointer to SPI flash object
 * @param[in] blockNumber	number of the block where to read the data
 * @param[out] data			pointer to data to be read
 * @param[in] size			number of bytes to be read
 * @param[in] offset		offset from beginning of block of first byte to be read
 *
 * @return SPIFLASH_SUCCESS if data is read successfully, SPIFLASH_ERROR otherwise
 */
SPIFlashStatus_t SPIFlashReadBlock(SPIFlash_t *SPIFlash, uint32_t blockNumber, uint8_t *data, uint32_t size, uint32_t offset);

#ifdef __cplusplus
}
#endif

#endif // __SPIFLASH_H__
