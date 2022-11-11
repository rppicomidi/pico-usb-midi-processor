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
#include "midi_processor_manager.h"
#include "rp2040_rtc.h"
rppicomidi::Settings_file::Settings_file() : vid{0}, pid{0}
{
    // Make sure the flash filesystem is working
    int error_code = pico_mount(false);
    if (error_code != 0) {
        printf("Error %s mounting the flash file system\r\nFormatting...", pico_errmsg(error_code));
        error_code = pico_mount(true);
        if (error_code != 0) {
            printf("Error %s mounting the flash file system\r\nFormatting...", pico_errmsg(error_code));
            exit(-1);
        }
        else {
            printf("completed\r\n");
        }
    }
    printf("successfully mounted flash file system\r\n");

    if (pico_unmount() != 0) {
        printf("Failed to unmount the flash file system\r\n");
    }
}

void rppicomidi::Settings_file::add_all_cli_commands(EmbeddedCli *cli)
{
    embeddedCliAddBinding(cli, {
        "format",
        "format settings file system",
        false,
        this,
        static_file_system_format
    });
    embeddedCliAddBinding(cli, {
        "fsstat",
        "display settings file system status",
        false,
        this,
        static_file_system_status
    });
    embeddedCliAddBinding(cli, {
        "ls",
        "list all settings files",
        false,
        this,
        static_list_files
    });
    embeddedCliAddBinding(cli, {
        "cat",
        "print file contents. usage: cat <fn>",
        true,
        this,
        static_print_file
    });
    embeddedCliAddBinding(cli, {
        "rm",
        "delete. usage: rm <fn>",
        true,
        this,
        static_delete_file
    });
    embeddedCliAddBinding(cli, {
        "fatcd",
        "change current directory. usage: fatcd <path>",
        true,
        this,
        static_fatfs_cd
    });
    embeddedCliAddBinding(cli, {
        "fatls",
        "list current directory. usage: fatls",
        false,
        this,
        static_fatfs_ls
    });
    embeddedCliAddBinding(cli, {
        "backup",
        "backup current presets. usage: backup",
        false,
        this,
        static_fatfs_backup
    });   
}

void rppicomidi::Settings_file::set_vid_pid(uint16_t vid_, uint16_t pid_)
{
    vid = vid_;
    pid = pid_;
}

int rppicomidi::Settings_file::load_settings_string(const char* settings_filename, char** raw_settings_ptr, bool mount)
{
    int error_code = 0;
    if (mount)
        error_code = pico_mount(false);
    if (error_code != 0) {
        printf("unexpected error %s mounting flash\r\n", pico_errmsg(error_code));
        return error_code;
    }
    int file = pico_open(settings_filename, LFS_O_RDONLY);
    if (file < 0) {
        // file isn't there
        if (mount)
            pico_unmount();
        return file; // the error code
    }
    else {
        // file is open. Read the whole contents to a string
        auto flen = pico_size(file);
        if (flen < 0) {
            // Something went wrong
            if (mount)
                pico_unmount();
            return flen;
        }
        // create a string long enough
        *raw_settings_ptr = new char[flen+1];
        if (!raw_settings_ptr) {
            pico_close(file);
            if (mount)
                pico_unmount();
            return LFS_ERR_NOMEM; // new failed
        }
        auto nread = pico_read(file, *raw_settings_ptr, flen);
        pico_close(file);
        if (mount)
            pico_unmount();
        if (nread == (lfs_size_t)flen) {
            // Success!
            (*raw_settings_ptr)[flen]='\0'; // just in case, add null termination
            return nread;
        }
        if (nread > 0) {
            delete[] *raw_settings_ptr;
            *raw_settings_ptr = nullptr;
            nread = LFS_ERR_IO;
        }
        return nread;
    }
}

int rppicomidi::Settings_file::load_settings_string(char** raw_settings_ptr)
{
    if (vid != 0 && pid != 0) {
        char settings_filename[]="0000-0000.json";
        get_filename(settings_filename);
        return load_settings_string(settings_filename, raw_settings_ptr);
    }
    return LFS_ERR_INVAL; // vid and pid are not valid yet
}

bool rppicomidi::Settings_file::load()
{
    char* raw_settings = NULL;
    auto error_code = load_settings_string(&raw_settings);
    bool result = false;
    if (error_code > 0) {
        //printf("load (%04x-%04x):\r\n", vid, pid);
        result = Midi_processor_manager::instance().deserialize(raw_settings);
    }
    else {
        char* default_raw_settings = Midi_processor_manager::instance().serialize_default();
        if (!default_raw_settings) {
            printf("Failed to allocate previous raw settings\r\n");
        }
        else {
            printf("loading defaults:\r\n");
            result = Midi_processor_manager::instance().deserialize(default_raw_settings);
            json_free_serialized_string(default_raw_settings);
        }        
    }
    if (error_code != LFS_ERR_OK)
        printf("settings file load error %s\r\n", pico_errmsg(error_code));
    if (raw_settings)
        delete[] raw_settings;
    return result;
}


FRESULT rppicomidi::Settings_file::backup_presets()
{
    FRESULT fatres = f_chdir("/");
    if (fatres != FR_OK)
        return fatres;
    lfs_dir_t dir;
    int err = pico_mount(false);
    if (err)
        return FR_INT_ERR;
    err = lfs_dir_open(&dir, "/");
    if (err) {
        pico_unmount();
        return FR_INT_ERR;
    }

    struct lfs_info info;
    bool flash_file_directory_ok = false;
    char dirname[30];
    while (true) {
        int res = lfs_dir_read(&dir, &info);
        if (res < 0) {
            pico_unmount();
            return FR_INT_ERR;
        }

        if (res == 0) {
            break;
        }
        // There is at least one settings file. Make sure there is a directory to store it in.
        if (!flash_file_directory_ok) {
            uint8_t month, day;
            uint16_t year;
            Rp2040_rtc::instance().get_date(year, month, day);
            sprintf(dirname, "/%02u-%02u-%04u", month, day, year);
            fatres = f_mkdir(dirname);
            uint16_t rev=1;
            while (fatres == FR_EXIST) {
                sprintf(&dirname[strlen(dirname)],"-%u", rev++);
                fatres = f_mkdir(dirname);
            }
            if (fatres != FR_OK) {
                lfs_dir_close(&dir);
                pico_unmount();
                return fatres;
            }
            flash_file_directory_ok = true;
        }
        fatres = f_chdir(dirname);
        if (fatres != FR_OK) {
            lfs_dir_close(&dir);
            pico_unmount();
            return fatres;
        }
        char* raw_settings;
        if(info.type == LFS_TYPE_REG) {
            int nread = load_settings_string(info.name, &raw_settings, false);
            if (nread > 0) {
                FIL bufile;
                fatres = f_open(&bufile, info.name, FA_CREATE_NEW | FA_WRITE);
                if (fatres != FR_OK) {
                    lfs_dir_close(&dir);
                    pico_unmount();
                    return fatres;
                }
                UINT written;
                fatres = f_write(&bufile, raw_settings, nread, &written);
                delete[] raw_settings;
                f_close(&bufile);
                if (fatres != FR_OK) {
                    lfs_dir_close(&dir);
                    pico_unmount();
                    return fatres;
                }
                printf("backed up preset %s\r\n", info.name);
            }
            else {
                delete[] raw_settings;
                lfs_dir_close(&dir);
                pico_unmount();
                return FR_INT_ERR;
            }
        }
    }

    err = lfs_dir_close(&dir);
    if (err) {
        pico_unmount();
        return FR_INT_ERR;
    }
    pico_unmount();
    return FR_OK;
}
bool rppicomidi::Settings_file::load_preset(uint8_t next_preset)
{
    char* raw_settings = NULL;
    auto error_code = load_settings_string(&raw_settings);
    bool result = false;
    if (error_code > 0) {
        printf("load (%04x-%04x):\r\n", vid, pid);
        result = Midi_processor_manager::instance().deserialize_preset(next_preset, raw_settings);
    }
    else {
        char* default_raw_settings = Midi_processor_manager::instance().serialize_default();
        if (!default_raw_settings) {
            printf("Failed to allocate previous raw settings\r\n");
        }
        else {
            printf("loading defaults:\r\n");
            result = Midi_processor_manager::instance().deserialize_preset(next_preset, default_raw_settings);
            json_free_serialized_string(default_raw_settings);
        }        
    }
    printf("settings file load error %s\r\n", pico_errmsg(error_code));
    if (raw_settings)
        delete[] raw_settings;
    return result;
}

int rppicomidi::Settings_file::store()
{
    // Get previous settings values
    char* previous_raw_settings = NULL;
    int error_code = load_settings_string(&previous_raw_settings);
    if (error_code < 0) {
        if (previous_raw_settings)
            delete[] previous_raw_settings;
        previous_raw_settings = Midi_processor_manager::instance().serialize_default();
        if (!previous_raw_settings) {
            printf("Failed to allocate previous raw settings\r\n");
            return error_code;
        }
    }
    // Serialize to a string for storage
    char* settings_str = Midi_processor_manager::instance().serialize(Midi_processor_manager::instance().get_current_preset(), previous_raw_settings);
    if (settings_str) {
        printf("store (%04x-%04x):\r\n",vid,pid);
    }
    else {
        return LFS_ERR_NOMEM;
    }
    // Move current settings file to a backup file if current settings file exists
    error_code = pico_mount(false);
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
        file = pico_open(settings_filename, LFS_O_WRONLY | LFS_O_CREAT);
        if (file < 0) {
            printf("error %s creating %s for writing\r\n", pico_errmsg(file), settings_filename);
            free((void*)settings_str);
            pico_unmount();
            return file;
        }
    }
    else {
        // file exists. Truncate it to 0 length in preparation for new data
        error_code = pico_close(file);
        if (error_code != LFS_ERR_OK) {
            printf("error %s closing %s\r\n", pico_errmsg(error_code), settings_filename);
            free((void*)settings_str);
            pico_unmount();
            return file;
        }

        file = pico_open(settings_filename, LFS_O_WRONLY);
        if (file < 0) {
            printf("error %s opening %s for writing\r\n", pico_errmsg(file), settings_filename);
            free((void*)settings_str);
            pico_unmount();
            return file;
        }
    }
    printf("writing settings to file %s\r\n", settings_filename);
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
        printf("store: Only %d bytes stored out of %u\r\n", error_code, strlen(settings_str));
        // hmm. no idea why that should happen
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}


int rppicomidi::Settings_file::lfs_ls(const char *path)
{
    lfs_dir_t dir;
    int err = lfs_dir_open(&dir, path);
    if (err) {
        return err;
    }

    struct lfs_info info;
    while (true) {
        int res = lfs_dir_read(&dir, &info);
        if (res < 0) {
            return res;
        }

        if (res == 0) {
            break;
        }

        switch (info.type) {
            case LFS_TYPE_REG: printf("reg "); break;
            case LFS_TYPE_DIR: printf("dir "); break;
            default:           printf("?   "); break;
        }

        static const char *prefixes[] = {"", "K", "M", "G"};
        for (int i = sizeof(prefixes)/sizeof(prefixes[0])-1; i >= 0; i--) {
            if (info.size >= static_cast<size_t>((1 << 10*i)-1)) {
                printf("%*lu%sB ", 4-(i != 0), info.size >> 10*i, prefixes[i]);
                break;
            }
        }

        printf("%s\n", info.name);
    }

    err = lfs_dir_close(&dir);
    if (err) {
        return err;
    }

    return 0;
}

void rppicomidi::Settings_file::static_file_system_format(EmbeddedCli*, char*, void*)
{
    printf("formatting settings file system then mounting it\r\n");
    int error_code = pico_mount(true);
    if (error_code != LFS_ERR_OK) {

    }
    else {
        error_code = pico_unmount();
        if (error_code != LFS_ERR_OK) {
            printf("unexpected error %s unmounting settings file system\r\n", pico_errmsg(error_code));
        }
        else {
            printf("File system successfully formated and unmounted\r\n");
        }
    }

}

void rppicomidi::Settings_file::static_file_system_status(EmbeddedCli*, char*, void*)
{
    int error_code = pico_mount(false);
    if (error_code != LFS_ERR_OK) {
        printf("can't mount settings file system\r\n");
        return;
    }
    struct pico_fsstat_t stat;
    if (pico_fsstat(&stat) == LFS_ERR_OK) {
        printf("FS: blocks %d, block size %d, used %d\n", (int)stat.block_count, (int)stat.block_size,
                (int)stat.blocks_used);
    }
    else {
        printf("could not read file system status\r\n");
    }
    error_code = pico_unmount();
    if (error_code != LFS_ERR_OK) {
        printf("can't unmount settings file system\r\n");
        return;
    }
}


void rppicomidi::Settings_file::static_list_files(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)args;
    auto me = reinterpret_cast<Settings_file*>(context);
    int error_code = pico_mount(false);
    if (error_code != LFS_ERR_OK) {
        printf("can't mount settings file system\r\n");
        return;
    }
    error_code = me->lfs_ls("/");
    if (error_code != LFS_ERR_OK) {
        printf("error listing path \"/\"\r\n");
    }
    error_code = pico_unmount();
    if (error_code != LFS_ERR_OK) {
        printf("can't unmount settings file system\r\n");
        return;
    }
}

static void print_fat_date(WORD wdate)
{
    uint16_t year = 1980 + ((wdate >> 9) & 0x7f);
    uint16_t month = (wdate >> 5) & 0xf;
    uint16_t day = wdate & 0x1f;
    printf("%02u/%02u/%04u\t", month, day, year);
}

static void print_fat_time(WORD wtime)
{
    uint8_t hour = ((wtime >> 11) & 0x1f);
    uint8_t min = ((wtime >> 5) & 0x3F);
    uint8_t sec = ((wtime &0x1f)*2);
    printf("%02u:%02u:%02u\t", hour, min, sec);
}

static FRESULT scan_files(const char* path)
{
    FRESULT res;
    DIR dir;
    //UINT i;
    static FILINFO fno;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            #if 0 // Do not do recursive scan.
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\r\n", path, fno.fname);
            }
            #endif
            printf("%lu\t",fno.fsize);
            print_fat_date(fno.fdate);
            print_fat_time(fno.ftime);
            printf("%s%c\r\n",fno.fname, (fno.fattrib & AM_DIR) ? '/' : ' ');
        }
        f_closedir(&dir);
    }

    return res;
}

void rppicomidi::Settings_file::static_fatfs_ls(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;
    FRESULT res = scan_files(".");
    if (res != FR_OK) {
        printf("Error %u listing files on drive\r\n", res);
    }
}

void rppicomidi::Settings_file::static_fatfs_backup(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;
    FRESULT res = instance().backup_presets();
    if (res != FR_OK) {
        printf("Error %u backing up files on drive\r\n", res);
    }
}

void rppicomidi::Settings_file::static_fatfs_cd(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    (void)context;
    uint16_t argc = embeddedCliGetTokenCount(args);
    FRESULT res;
    char temp_cwd[256] = {'/', '\0'};
    if (argc == 0) {
        res = f_chdir(temp_cwd);
    }
    else if (argc == 1) {
        strncpy(temp_cwd, embeddedCliGetToken(args, 1), sizeof(temp_cwd)-1);
        temp_cwd[sizeof(temp_cwd)-1] = '\0';
        res = f_chdir(temp_cwd);
    }
    else {
        printf("usage: fatcd <new path>\r\n");
        return;
    }
    if (res != FR_OK) {
        printf("error %u setting cwd to %s\r\n", res, temp_cwd);
    }
    res = f_getcwd(temp_cwd, sizeof(temp_cwd));
    if (res == FR_OK)
        printf("cwd=\"%s\"\r\n", temp_cwd);
    else
        printf("error %u getting cwd\r\n", res);
}
void rppicomidi::Settings_file::static_print_file(EmbeddedCli* cli, char* args, void* context)
{
    (void)cli;
    auto me = reinterpret_cast<Settings_file*>(context);
    if (embeddedCliGetTokenCount(args) != 1) {
        printf("usage: cat <filename>");
        return;
    }
    const char* fn=embeddedCliGetToken(args, 1);
    char* raw_settings;
    int error_code = me->load_settings_string(fn, &raw_settings);
    if (error_code > 0) {
        printf("File: %s\r\n%s\r\n", fn, raw_settings);
        delete[] raw_settings;
    }
    else {
        switch(error_code) {
        case LFS_ERR_NOENT:
            printf("File %s does not exist\r\n", fn);
            break;
        case LFS_ERR_ISDIR:
            printf("%s is a directory\r\n",fn);
            break;
        case LFS_ERR_CORRUPT:
            printf("%s is corrupt\r\n",fn);
            break;
        case LFS_ERR_IO:
            printf("IO Error reading %s\r\n", fn);
            break;
        default:
            printf("Unexpected Error %s reading %s\r\n", pico_errmsg(error_code), fn);
            break;
        }
    }
}

void rppicomidi::Settings_file::static_delete_file(EmbeddedCli* cli, char* args, void*)
{
    (void)cli;
    if (embeddedCliGetTokenCount(args) != 1) {
        printf("usage: rm <filename>");
        return;
    }
    const char* fn=embeddedCliGetToken(args, 1);
    int error_code = pico_mount(false);
    if (error_code == LFS_ERR_OK) {
        error_code = pico_remove(fn);
        if (error_code != LFS_ERR_OK) {
            switch(error_code) {
            case LFS_ERR_NOENT:
                printf("File %s does not exist\r\n", fn);
                break;
            case LFS_ERR_ISDIR:
                printf("%s is a directory\r\n",fn);
                break;
            case LFS_ERR_CORRUPT:
                printf("%s is corrupt\r\n",fn);
                break;
            case LFS_ERR_IO:
                printf("IO Error deleting %s\r\n", fn);
                break;
            default:
                printf("Unexpected Error %s deleting %s\r\n", pico_errmsg(error_code), fn);
                break;
            }
        }
        error_code = pico_unmount();
        if (error_code != LFS_ERR_OK) {
            printf("Unexpected Error %s unmounting settings file system\r\n", pico_errmsg(error_code));
        }
    }
}
