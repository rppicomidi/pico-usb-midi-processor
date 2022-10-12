/**
 * @file mc_bridge_model.h
 * @brief this class is the MVC model for the configuration UI
 *
 * This class contains all the settings required by the Pico_usb_midi_processor
 * class, which is the controller in the MVC model.
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
namespace rppicomidi {
class Midi_processor_model {
public:
    Midi_processor_model();

    void set_num_cables(uint8_t num_cables_in_, uint8_t num_cables_out_) {
        num_cables_in = num_cables_in_;
        num_cables_out = num_cables_out_;
    }

    void get_num_cables(uint8_t& num_cables_in_, uint8_t& num_cables_out_) {
        num_cables_in_ = num_cables_in;
        num_cables_out_ = num_cables_out;
    }

    /**
     * @brief add callback function to the list of callback functions
     * to call when any setting changes
     *
     * @param callback a function to call when any setting changes
     */

    void subscribe_settings_changed(void (*callback)());

    /**
     * @brief delete callback function from the list of callback functions
     * to call when any setting changes
     *
     * @param callback the function to remove
     */
    uint32_t unsubscribe_settings_changed(void (*callback)());

    /**
     * @brief set the current settings to the default settings
     *
     */
    void load_defaults();

    /**
     * @brief
     * 
     * @return true if need to store settings
     */
    bool get_needs_save() {return needs_save; }

    /**
     * @brief clear the needs_save flag (e.g., after storing settings to flash)
     */
    void clear_needs_save() {needs_save = false; }

    /**
     * @brief Serialize the model to a JSON C-style string
     *
     * @return the JSON formatted null-terminated string
     */
    char* serialize() const;

    /**
     * @brief deserialize the JSON encoded C-style null terminated settings
     * string to the model
     *
     * @param settings_str JSON encoded C-style null terminated settings string
     * @return true if the JSON string was successfully parsed
     *
     * @note any invalid value in settings_str will cause that model value to
     * be set to default.
     */
    bool deserialize(const char* settings_str);
private:
    bool needs_save;    // true if the settings have been modified since the last save (not serialized here)
    uint8_t num_cables_in;
    uint8_t num_cables_out;
};
}
