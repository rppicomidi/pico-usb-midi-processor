/*-----------------------------------------------------------------------/
/  Low level disk interface modlue include file   (C)ChaN, 2019          /
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED
#include "tusb.h"
#include "pico/multicore.h"
#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;


/*---------------------------------------*/
/* Prototypes for disk control functions */


DSTATUS disk_initialize (BYTE pdrv);
DSTATUS disk_status (BYTE pdrv);
DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);
DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);


/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl fucntion */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC			0	/* Complete pending write process (needed at FF_FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at FF_USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at FF_USE_MKFS == 1) */
#define CTRL_TRIM			4	/* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */

/* Generic command (Not used by FatFs) */
#define CTRL_POWER			5	/* Get/Set power status */
#define CTRL_LOCK			6	/* Lock/Unlock media removal */
#define CTRL_EJECT			7	/* Eject media */
#define CTRL_FORMAT			8	/* Create physical format on the media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */
#define ISDIO_READ			55	/* Read data form SD iSDIO register */
#define ISDIO_WRITE			56	/* Write data to SD iSDIO register */
#define ISDIO_MRITE			57	/* Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command */
#define ATA_GET_REV			20	/* Get F/W revision */
#define ATA_GET_MODEL		21	/* Get model name */
#define ATA_GET_SN			22	/* Get serial number */

/* Helper functions for managing USB FAT drives in tinyusb */
typedef enum {MSC_FAT_IN_PROGRESS, MSC_FAT_COMPLETE, MSC_FAT_ERROR} msc_fat_xfer_status_t;

/**
 * @brief set the status to drive unplugged
 * 
 * @param pdrv the physical drive number
 */
void msc_fat_unplug(BYTE pdrv);

/**
 * @brief set the status to drive present
 * 
 * @param pdrv the physical drive number
 */
void msc_fat_plug_in(BYTE pdrv);

/**
 * @brief check if the drive is plugged in
 * 
 * @param pdrv the physical drive number
 * @return true if the drive is plugged in
 * @return false if not plugged in
 */
bool msc_fat_is_plugged_in(BYTE pdrv);

/**
 * @brief initialize the diskio module for use with the MSC
 */
void msc_fat_init();

/**
 * @brief set the current transfer status
 * 
 * @param stat the transfer status
 */
void msc_fat_set_status(msc_fat_xfer_status_t stat);

/**
 * @brief get the current transfer status
 * 
 * @return msc_fat_xfer_status_t the current transfer status
 */
msc_fat_xfer_status_t msc_fat_get_xfer_status();

/**
 * @brief wait for the current MSC transfer to complete
 * 
 */
void msc_fat_wait_transfer_complete();

/**
 * @brief callback when the current pending MSC transfer is complete
 * 
 * @param dev_addr the address of the attached MSC device
 * @param cbw the command block wrapper structure
 * @param csw the command status wrapper structure
 * @return true if transfer was successful
 * @return false if the transfer failed or there was a phase error
 */
bool msc_fat_complete_cb(uint8_t dev_addr, msc_cbw_t const* cbw, msc_csw_t const* csw);

/**
 * @brief The task function for the main() function "superloop"
 *
 * This function must call tuh_task() and may call whatever functions
 * are required to make the main loop feel responsive whilst
 * msc_fat_wait_transfer_complete() blocks for completed transfers
 */
void main_loop_task();
#ifdef __cplusplus
}
#endif

#endif
