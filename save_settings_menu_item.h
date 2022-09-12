/**
 * @file save_settings_menu_item.h
 * @brief this class describes a menu item that is a line of text
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
#include "quit_text_menu_item.h"
#include "settings_file.h"
namespace rppicomidi {
class Save_settings_menu_item : public Quit_text_menu_item
{
public:
    Save_settings_menu_item(Mono_graphics& screen_, uint8_t x_, uint8_t y_, const Mono_mono_font& font_, std::string text_, Settings_file settings_) :
        Quit_text_menu_item{screen_, x_, y_, font_, text_}, settings{settings_} {}

    virtual void entry() {
        printf("Saving settings...");
        settings.store();
        printf("done\r\n");
    }
private:
    Settings_file& settings;
};
}