/**
 * @file settings_file.h
 * @brief this class is the MVC model for the configuration UI
 *
 * This class manages loading and storing JSON formatted settings
 * strings to files named VVVV-PPPP.json or VVVV-PPPP.bu, where
 * VVVV is the idVendor in hex, and PPPP is the idProduct, in hex,
 * of the connected device
 *
 * MIT License
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

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "littlefs-lib/pico_hal.h"
#include "embedded_cli.h"
#include "ff.h"

namespace rppicomidi {
/**
 * @brief convert an integer type to a null-terminated C-string in hex notation
 *
 * @param w the value to convert to hex
 * @param hex_cstr a pointer to the buffer to receive the converted string
 * @param max_cstr the maximum characters to convert. If max_cstr is not
 * long enough to contain all the required hex digits, this function asserts.
 * If max_cstr is not large enough to contain all the hex digits plus a null
 * termination, then the null-termination is omitted.
 *
 * @note this template modified from https://stackoverflow.com/questions/5100718/integer-to-hex-string-in-c
 */
template <typename I> void n2hexstr(I w, char* hex_cstr, size_t max_cstr) {
    size_t hex_len = sizeof(I)<<1;
    assert(max_cstr >= hex_len);
    static const char* digits = "0123456789ABCDEF";
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        hex_cstr[i] = digits[(w>>j) & 0x0f];
    if (hex_len < max_cstr)
        hex_cstr[hex_len] = '\0';
}

class Settings_file {
public:
    // Singleton Pattern

    /**
     * @brief Get the Instance object
     *
     * @return the singleton instance
     */
    static Settings_file& instance()
    {
        static Settings_file _instance; // Guaranteed to be destroyed.
                                        // Instantiated on first use.
        return _instance;
    }
    Settings_file(Settings_file const&) = delete;
    void operator=(Settings_file const&) = delete;

    void add_all_cli_commands(EmbeddedCli *cli);
    /**
     * @brief Set the vid and pid values (used to form the JSON key
     * for serializing this object). This will trigger loading
     * settings if settings for this device exist, or it will
     * create a new settings object in flash initialized to defaults
     *
     * @param vid_ the connected device idVendor
     * @param pid_ the connected device idProduct
     */
    void set_vid_pid(uint16_t vid_, uint16_t pid_);

    /**
     * @brief load settings from flash based on the connected device's USB vid & pid
     *
     * @return true if successful or false if there was a problem loading settings
     */
    bool load();

    bool load_preset(uint8_t);

    /**
     * @brief If the current settings are different from the settings in flash,
     * write the settings to the flash
     *
     * @return 0 if successful, an negative number if there was an error and
     * a positive number if the number of bytes stored does not match the expected
     * number of bytes
     * @note this function will disable interrupts while it is writing to flash.
     */
    int store();

    /**
     * @brief set buffer pointed to by fn to a null terminated
     * C-style character string VVVV-PPPP, where
     * VVVV is the connected device's idVendor and PPPP is the
     * connected device's idProduct
     *
     * @param fn points to a character buffer that will be set
     * to the null terminated VVVV-PPPP C-style character string.
     * The fn buffer must be at least 10 bytes long or memory
     * corruption will result.
     */
    void get_filename(char* fn) {
        n2hexstr<uint16_t>(vid, fn, 4);
        n2hexstr<uint16_t>(pid, fn+5, 4);
        fn[4] = '-';
    }

    /**
     * @brief copy all presets of all devices stored in the local file system
     * to external flash drive
     * 
     * The presets will be stored in a directory named MM-DD-YYYY-HH-MM-SS
     * @return FR_OK if no error, an error code otherwise 
     */
    FRESULT backup_all_presets();

    /**
     * @brief copy preset(s) specified in the backup path to local storage
     * 
     * @param backup_path if restoring all presets backed up, the backup_path
     * will be a string of the form returned by get_next_backup_directory_name().
     * If restoring a all presets for a single device preset, it will be in
     * the form of a directory name as return by 
     * get_next_backup_directory_name()+"/"+"vid-pid.json"
     * @return FRESULT 
     */
    FRESULT restore_presets(const char* backup_path);

    /**
     * @brief Get the next backup directory name
     * 
     * @param dirname the character buffer where the next directory name is stored
     * @param maxname the length of the dirname buffer
     * @return true if a valid backup directory name is in dirname, false otherwise
     */
    bool get_next_backup_directory_name(char* dirname, size_t maxname);

    bool get_all_preset_directory_names(std::vector<std::string>& dirname_list);
    bool get_all_preset_filenames(const char* directory_name, std::vector<std::string>& filename_list);

    /**
     * @brief Read the product string
     *
     * @param directory The backup directory under base_preset_path
     * @param filename The file name of the file under the backup directory
     * @param prod_string a pointer to storage large enough to hold the product string
     * @param max_string the maximum number of bytes in the string including the null character
     * @return true if the product string was read, false otherwise
     */
    bool get_setting_file_product_string(const char* directory, const char* file, char* prod_string, size_t max_string);
private:
    Settings_file();

    /**
     * @brief get the JSON serialized string from the file named settings_filename
     *
     * @param raw_settings_ptr 
     * @return int the number of bytes in the settings string, or a negative
     * LFS error code if there was an error.
     */
    int load_settings_string(char** raw_settings_ptr);

    int load_settings_string(const char* fn, char** raw_settings_ptr, bool mount=true);

    FRESULT restore_one_file(const char* restore_path, const char* filename);

    /**
     * @brief See https://github.com/littlefs-project/littlefs/issues/2
     * 
     * @param lfs pointer to lfs object
     * @param path root path string
     * @return int 0 if no error, < 0 if there is an error
     */
    int lfs_ls(const char *path);

    static void static_file_system_format(EmbeddedCli*, char*, void*);
    static void static_file_system_status(EmbeddedCli*, char*, void*);
    static void static_list_files(EmbeddedCli* cli, char* args, void* context);
    static void static_print_file(EmbeddedCli* cli, char* args, void* context);
    static void static_delete_file(EmbeddedCli* cli, char* args, void*);
    static void static_fatfs_cd(EmbeddedCli* cli, char* args, void*);
    static void static_fatfs_ls(EmbeddedCli* cli, char* args, void*);
    static void static_fatfs_backup(EmbeddedCli* cli, char* args, void*);
    static void static_fatfs_restore(EmbeddedCli* cli, char* args, void*);
    FRESULT scan_files(const char* path);
    void print_fat_time(WORD wtime);
    void print_fat_date(WORD wtime);
    FRESULT create_preset_backup_directory(char* dirname);
    uint16_t vid;       // the idVendor of the connected device (not serialized here)
    uint16_t pid;       // the idProduct of the connected device (not serialized here)
    static constexpr const char* base_preset_path = "/rppicomidi-pico-usb-midi-processor";
};
}