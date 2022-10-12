/**
 * @file home_screen.h
 * @brief This class implements the pico-usb-midi-processor UI home screen
 * 
 * Copyright (c) 2022 rppicomidi
 * 
 * The MIT License (MIT)
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
#include "pico/mutex.h"
#include "mono_graphics_lib.h"
#include "view_manager.h"
#include "menu.h"
#include "view_launch_menu_item.h"
#include "midi_processor_setup_screen.h"
namespace rppicomidi {
class Home_screen : public View
{
public:

    Home_screen(View_manager& view_manager_, Mono_graphics& screen_, const char* device_label_);

    virtual ~Home_screen()=default;

    void draw() final;
    Select_result on_select(View** new_view) final;
    void on_increment(uint32_t delta) final {menu.on_increment(delta); };
    void on_decrement(uint32_t delta) final {menu.on_decrement(delta); };

    /**
     * @brief set the device_label string to the new value
     * 
     * @param device_label_ A pointer to the new device label (C-style) string
     * @param num_in_cables_ The number of virtual MIDI IN cables of the connected device
     * @param num_out_cables_ The number of virtual MIDI_OUT cables of the connected device
     *
     * @note the device_label will be truncated to max_device_label characters
     */
    void set_connected_device(const char* device_label_, uint8_t num_in_cables_, uint8_t num_out_cables_);
private:
    // Get rid of default constructor and copy constructor
    Home_screen() = delete;
    Home_screen(Home_screen&) = delete;

    View_manager& view_manager;
    const Mono_mono_font& label_font;
    Menu menu;
    static const uint8_t max_line_length = 21;
    static const uint8_t max_device_label = max_line_length * 2;
    char device_label[max_device_label+1];
    uint8_t num_in_cables;
    uint8_t num_out_cables;
    std::vector<Midi_processor_setup_screen*> midi_in_setup;
    std::vector<Midi_processor_setup_screen*> midi_out_setup;
};
}