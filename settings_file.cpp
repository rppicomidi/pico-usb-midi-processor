#include <assert.h>
#include "settings_file.h"
#include "littlefs-lib/pico_hal.h"

rppicomidi::Settings_file::Settings_file(Midi_processor_model& model_) : model{model_}, vid{0}, pid{0}
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
        char settings_filename[15];
        get_filename(settings_filename);
        int file = pico_open(settings_filename, LFS_O_RDONLY);
        if (file < 0) {
            // file isn't there
            pico_unmount();
            return file;
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
            if (nread == flen) {
                // Success!
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
    //using json = nlohmann::json;
    char* raw_settings = NULL;
    auto error_code = load_settings_string(&raw_settings);
    if (error_code == LFS_ERR_OK) {
        printf("load (%04x-%04x): %s\r\n", vid, pid, raw_settings);
        bool result = model.deserialize(raw_settings);
        free((void *)raw_settings);
        return result;
    }
    if (raw_settings)
        free((void *)raw_settings);
    return false;
}

int rppicomidi::Settings_file::store()
{
    char* raw_settings;

    // Serialize to a string for storage
    char* settings_str = model.serialize();
    if (settings_str) {
        printf("store (%04x-%04x): %s\r\n",vid,pid, settings_str);
    }
    else {
        return false;
    }
    // Move current settings file to a backup file if current settings file exists
    int error_code = pico_mount(false);
    if (error_code != 0) {
        free((void*)settings_str);
        printf("unexpected error %d mounting flash\r\n", error_code);
        return false;
    }

    char settings_filename[]="0000-0000.json";
    get_filename(settings_filename);
    int file = pico_open(settings_filename, LFS_O_RDONLY);
    if (file < 0) {
        // file does not exist
        file = pico_open(settings_filename, LFS_O_RDWR | LFS_O_CREAT);
        if (file < 0) {
            printf("could not open settings.json for writing\r\n");
            free((void*)settings_str);
            pico_unmount();
            return false;
        }
    }
    else {
        // file exists. Copy it to a backup
        pico_close(file);
        char backup_filename[]="0000-0000.bu";
        get_filename(settings_filename);
        if (pico_rename(settings_filename, backup_filename) == LFS_ERR_EXIST) {
            if (pico_remove(backup_filename) == LFS_ERR_OK) {
                if (pico_rename(settings_filename, backup_filename) != LFS_ERR_OK) {
                    free((void*)settings_str);
                    // something went wrong
                    pico_unmount();
                    return false;
                }
            }
            else {
                printf("error moving %s to s%s\r\n",settings_filename, backup_filename);
                free((void*)settings_str);
                pico_unmount();
                return false;
            }
        } 
        file = pico_open(settings_filename, LFS_O_RDWR | LFS_O_CREAT);
        if (file < 0) {
            printf("could not open %s for writing\r\n",settings_filename);
            free((void*)settings_str);
            pico_unmount();
            return false;
        }
    }
    // file is open for writing; write the settings
    error_code = pico_write(file, settings_str, strlen(settings_str)+1);

    free(settings_str);
    pico_close(file);
    pico_unmount();
    if (error_code < 0) {
        return error_code;
    }
    else if ((size_t)error_code < strlen(settings_str)) {
        printf("store: WTF error_code=%d strlen(settings_str)=%u\r\n", error_code, strlen(settings_str));
        // hmm. no idea why that should happen
        return LFS_ERR_IO;
    }
    return LFS_ERR_OK;
}
