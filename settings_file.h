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
#include "midi_processor_model.h"

namespace rppicomidi {
/**
 * @brief convert an integer type to a std::string in hex notation
 *
 * @note this template modified from https://stackoverflow.com/questions/5100718/integer-to-hex-string-in-c
 */
template <typename I> void n2hexstr(I w, char* hex_cstr, size_t max_cstr) {
    size_t hex_len = sizeof(I)<<1;
    assert(max_cstr > hex_len);
    static const char* digits = "0123456789ABCDEF";
    for (size_t i=0, j=(hex_len-1)*4 ; i<hex_len; ++i,j-=4)
        hex_cstr[i] = digits[(w>>j) & 0x0f];
    hex_cstr[hex_len] = '\0';
}

class Settings_file {
public:
    Settings_file(Midi_processor_model& model_);
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

    /**
     * @brief If the current settings are different from the settings in flash,
     * write the settings to the flash
     *
     * @return true if settings in flash match the current Model
     * @note this function will disable interrupts while it is writing to flash.
     */
    int store();
private:
    /**
     * @brief get the JSON serialized string from the file named settings_filename
     *
     * @param raw_settings_ptr 
     * @return int 
     */
    int load_settings_string(char** raw_settings_ptr);

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
        n2hexstr<uint16_t>(vid, fn, 5);
        n2hexstr<uint16_t>(pid, fn+5, 5);
        fn[4] = '-';
    }

    Midi_processor_model& model;
    uint16_t vid;       // the idVendor of the connected device (not serialized here)
    uint16_t pid;       // the idProduct of the connected device (not serialized here)
};
}