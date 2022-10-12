/* MIT License
 *
 * Copyright (c) 2022 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <assert.h>
#include "settings_file.h"
#include "littlefs-lib/pico_hal.h"
#include "midi_processor_manager.h"
rppicomidi::Settings_file::Settings_file() : vid{0}, pid{0}
{
    // Make sure the flash filesystem is working
    int error_code = pico_mount(false);
    if (error_code != 0) {
        printf("Error %d mounting the flash file system\r\nFormatting...",error_code);
        error_code = pico_mount(true);
        if (error_code != 0) {
            printf("Error %d mounting the flash file system\r\nFormatting...",error_code);
            exit(-1);
        }
        else {
            printf("completed\r\n");
        }
    }
    printf("successfully mounted flash file system\r\n");
    struct pico_fsstat_t stat;
    if (pico_fsstat(&stat) == 0) {
        printf("FS: blocks %d, block size %d, used %d\n", (int)stat.block_count, (int)stat.block_size,
                (int)stat.blocks_used);
    }
    if (pico_unmount() != 0) {
        printf("Failed to unmount the flash file system\r\n");
    }
}

void rppicomidi::Settings_file::set_vid_pid(uint16_t vid_, uint16_t pid_)
{
    vid = vid_;
    pid = pid_;
}


int rppicomidi::Settings_file::load_settings_string(char** raw_settings_ptr)
{
    if (vid != 0 && pid != 0) {
        int error_code = pico_mount(false);
        if (error_code != 0) {
            printf("unexpected error %d mounting flash\r\n", error_code);
            return false;
        }
        char settings_filename[]="0000-0000.json";
        get_filename(settings_filename);
        int file = pico_open(settings_filename, LFS_O_RDONLY);
        if (file < 0) {
            // file isn't there
            pico_unmount();
            return file; // the error code
        }
        else {
            // file is open. Read the whole contents to a string
            auto flen = pico_size(file);
            if (flen < 0) {
                // Something went wrong
                pico_unmount();
                return flen;
            }
            // create a string long enough
            *raw_settings_ptr = new char[flen+1];
            if (!raw_settings_ptr) {
                pico_close(file);
                pico_unmount();
                return LFS_ERR_NOMEM; // new failed
            }
            auto nread = pico_read(file, *raw_settings_ptr, flen);
            pico_close(file);
            pico_unmount();
            if (nread == (lfs_size_t)flen) {
                // Success!
                (*raw_settings_ptr)[flen]='\0'; // just in case, add null termination
                return LFS_ERR_OK;
            }
            if (nread > 0) {
                delete[] *raw_settings_ptr;
                *raw_settings_ptr = nullptr;
                nread = LFS_ERR_IO;
            }
            return nread;
        }
    }
    return LFS_ERR_INVAL; // vid and pid are not valid yet
}

bool rppicomidi::Settings_file::load()
{
    char* raw_settings = NULL;
    auto error_code = load_settings_string(&raw_settings);
    if (error_code == LFS_ERR_OK) {
        printf("load (%04x-%04x): %s\r\n", vid, pid, raw_settings);
        bool result = Midi_processor_manager::instance().deserialize(raw_settings);
        delete[] raw_settings;
        return result;
    }
    printf("settings file load error %s\r\n", pico_errmsg(error_code));
    if (raw_settings)
        delete[] raw_settings;
    return false;
}

int rppicomidi::Settings_file::store()
{
    // Serialize to a string for storage
    char* settings_str = Midi_processor_manager::instance().serialize();
    if (settings_str) {
        printf("store (%04x-%04x): %s\r\n",vid,pid, settings_str);
    }
    else {
        return LFS_ERR_NOMEM;
    }
    // Move current settings file to a backup file if current settings file exists
    int error_code = pico_mount(false);
    if (error_code != 0) {
        free((void*)settings_str);
        printf("unexpected error %s mounting flash\r\n", pico_errmsg(error_code));
        return error_code;
    }

    char settings_filename[]="0000-0000.json";
    get_filename(settings_filename);
    printf("opening file %s for read only\r\n", settings_filename);
    int file = pico_open(settings_filename, LFS_O_RDONLY);
    if (file < 0) {
        // file does not exist
        printf("opening/creating file %s for read/write\r\n", settings_filename);
        file = pico_open(settings_filename, LFS_O_RDWR | LFS_O_CREAT);
        if (file < 0) {
            printf("could not open settings.json for writing\r\n");
            free((void*)settings_str);
            pico_unmount();
            return file;
        }
    }
    else {
        // file exists. Copy it to a backup
        pico_close(file);
        char backup_filename[]="0000-0000.bu";
        get_filename(backup_filename);
        printf("renaming file %s to file %s\r\n", settings_filename, backup_filename);
        error_code = pico_rename(settings_filename, backup_filename);
        if (error_code == LFS_ERR_EXIST) {
            printf("backup file %s exists. Deleting it first\r\n", backup_filename);
            if (pico_remove(backup_filename) == LFS_ERR_OK) {
                error_code = pico_rename(settings_filename, backup_filename);
                if (error_code != LFS_ERR_OK) {
                    printf("error %s renaming file %s to file %s\r\n", pico_errmsg(error_code), settings_filename, backup_filename);
                    free((void*)settings_str);
                    // something went wrong
                    pico_unmount();
                    return error_code;
                }
            }
            else {
                printf("error moving %s to s%s\r\n",settings_filename, backup_filename);
                free((void*)settings_str);
                pico_unmount();
                return error_code;
            }
        }
        else if (error_code != LFS_ERR_OK) {
            printf("error moving %s to s%s\r\n",settings_filename, backup_filename);
            free((void*)settings_str);
            pico_unmount();
            return false;
        }
        printf("opening/creating file %s for read/write\r\n", settings_filename);
        file = pico_open(settings_filename, LFS_O_RDWR | LFS_O_CREAT);
        if (file < 0) {
            printf("could not open %s for writing\r\n",settings_filename);
            free((void*)settings_str);
            pico_unmount();
            return file;
        }
    }
    printf("writing\r\n%s\r\nto file %s\r\n", settings_str, settings_filename);
    // file is open for writing; write the settings
    error_code = pico_write(file, settings_str, strlen(settings_str)+1);

    free(settings_str);
    pico_close(file);
    pico_unmount();
    if (error_code < 0) {
        printf("error %s writing settings to file\r\n", pico_errmsg(error_code));
        return error_code;
    }
    else if ((size_t)error_code < strlen(settings_str)) {
        printf("store: WTF error_code=%d strlen(settings_str)=%u\r\n", error_code, strlen(settings_str));
        // hmm. no idea why that should happen
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}
