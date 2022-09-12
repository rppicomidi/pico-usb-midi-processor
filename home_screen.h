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
#include "mono_graphics_lib.h"
#include "view_manager.h"
#include "setup_menu.h"

namespace rppicomidi {
class Home_screen : public View
{
public:
    /**
     * @brief Construct a new Home_screen object
     * 
     * @param screen_ 
     * @param device_label_ A pointer to the initial device label string
     * @param setup_menu_ a referene to the Setup_menu object
     */
    Home_screen(View_manager& view_manager_, Mono_graphics& screen_,
                const char* device_label_, Setup_menu& setup_menu_);

    virtual ~Home_screen()=default;

    void draw() final;
    Select_result on_select() final;

    /**
     * @brief set the device_label string to the new value
     * 
     * @param device_label_ A pointer to the new device label (C-style) string
     *
     * @note the device_label will be truncated to max_device_label characters
     */
    void set_device_label(const char* device_label_);
private:
    // Get rid of default constructor and copy constructor
    Home_screen() = delete;
    Home_screen(Home_screen&) = delete;
    void format_device_label();
    void center_text(const char* text_, uint8_t y_);
    View_manager& view_manager;
    const Mono_mono_font& label_font;
    Setup_menu& setup_menu;
    static const uint8_t max_line_length = 21;
    static const uint8_t max_device_label = max_line_length * 2;
    char device_label[max_device_label+1];
};
}