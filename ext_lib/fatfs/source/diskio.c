/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org) (for original example code)
 * Copyright (c) 2022 rppicomidi (for porting to FatFs 14b and Pico-USB-PIO project)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This diskio implementation assumes that all calls to the FatFs API are made from
 * RP2040 core 0 and the MSC USB host stack is running on core 1. Otherwise, the
 * API needs some mechanism to block until the USB transfers between the USB drive
 * and the RP2040 complete (e.g., an RTOS or manually task calls in a loop)
 */

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "tusb.h"
#include "pico/mutex.h"
#if CFG_TUH_MSC

static DSTATUS disk_state[CFG_TUH_DEVICE_MAX];
static mutex_t mmc_fat_mutex;

static  mmc_fat_xfer_status_t mmc_fat_status;
/*-----------------------------------------------------------------------*/
/* MMC plug status functions                                             */
/*-----------------------------------------------------------------------*/
void mmc_fat_unplug(
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    if (pdrv < CFG_TUH_MSC)
        disk_state[pdrv] |= STA_NOINIT | STA_NODISK;;
}

void mmc_fat_plug_in(
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    if (pdrv < CFG_TUH_MSC)
        disk_state[pdrv] &= ~STA_NODISK;
}

bool mmc_fat_is_plugged_in(
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    bool plugged_in = false;
    if (pdrv < CFG_TUH_MSC) {
        plugged_in = (disk_state[pdrv] & STA_NODISK) == 0;
    }
    return plugged_in;
}

void mmc_fat_init()
{
    mutex_init(&mmc_fat_mutex);
    mmc_fat_status = MMC_FAT_ERROR;
    for (int pdrv=0;pdrv<CFG_TUH_DEVICE_MAX;pdrv++)
        mmc_fat_unplug(pdrv); // assume no drives are plugged int
}

void mmc_fat_set_status(mmc_fat_xfer_status_t stat)
{
    mutex_enter_blocking(&mmc_fat_mutex);
    mmc_fat_status = stat;
    mutex_exit(&mmc_fat_mutex);
}

mmc_fat_xfer_status_t mmc_fat_get_xfer_status()
{
    mutex_enter_blocking(&mmc_fat_mutex);
    mmc_fat_xfer_status_t res = mmc_fat_status;
    mutex_exit(&mmc_fat_mutex);
    return res;
}

void mmc_fat_wait_transfer_complete()
{
    while(mmc_fat_get_xfer_status() == MMC_FAT_IN_PROGRESS) {
    }
}

bool mmc_fat_complete_cb(uint8_t dev_addr, msc_cbw_t const* cbw, msc_csw_t const* csw)
{
    (void)dev_addr;
    (void)cbw;
    if (csw->status == MSC_CSW_STATUS_PASSED) {
        mmc_fat_set_status(MMC_FAT_COMPLETE);
    }
    else {
        mmc_fat_set_status(MMC_FAT_ERROR);
    }
    return csw->status == MSC_CSW_STATUS_PASSED;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    if (pdrv >= CFG_TUH_DEVICE_MAX)
        return STA_NOINIT | STA_NODISK;
    return disk_state[pdrv];
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
    if (pdrv < CFG_TUH_DEVICE_MAX) {
        if ((disk_state[pdrv] & STA_NODISK) == 0) {
            disk_state[pdrv] = 0;
        }
    }

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res = RES_PARERR;
    if (pdrv < CFG_TUH_DEVICE_MAX && buff != NULL) {
        if (disk_state[pdrv] & (STA_NODISK | STA_NOINIT)) {
            res = RES_NOTRDY;
        }
        else {
            assert(mmc_fat_get_xfer_status() == MMC_FAT_COMPLETE);
            uint8_t dev_addr = pdrv+1;
            mmc_fat_set_status(MMC_FAT_IN_PROGRESS);
            if (!tuh_msc_read10(dev_addr, 0, buff, sector, count, mmc_fat_complete_cb)) {
                res = RES_ERROR;
            }
            else {
                mmc_fat_wait_transfer_complete();
                if (mmc_fat_get_xfer_status() == MMC_FAT_ERROR) {
                    res = RES_ERROR;
                }
                else {
                    res = RES_OK;
                }
            }
        }
    }
    return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res = RES_PARERR;
    if (pdrv < CFG_TUH_DEVICE_MAX && buff != NULL) {
        if (disk_state[pdrv] & (STA_NODISK | STA_NOINIT)) {
            res = RES_NOTRDY;
        }
        else {
            assert(mmc_fat_get_xfer_status() == MMC_FAT_COMPLETE);
            uint8_t dev_addr = pdrv+1;
            mmc_fat_set_status(MMC_FAT_IN_PROGRESS);
            if (!tuh_msc_write10(dev_addr, 0, buff, sector, count, mmc_fat_complete_cb)) {
                res = RES_ERROR;
            }
            else {
                mmc_fat_wait_transfer_complete();
                if (mmc_fat_get_xfer_status() == MMC_FAT_ERROR) {
                    res = RES_ERROR;
                }
                else {
                    res = RES_OK;
                }
            }
        }
    }
    return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    (void)buff;
    (void)pdrv;

    if (cmd != CTRL_SYNC)
        return RES_ERROR;
    return RES_OK;
}

#endif