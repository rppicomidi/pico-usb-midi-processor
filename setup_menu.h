/**
 * @file setup_menu.h
 * @brief this class describes a menu item that can be incremented or decremented
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
#include "menu.h"
#include "quit_text_menu_item.h"
#include "save_settings_menu_item.h"
#include "int_spinner.h"
#include "midi_processor_model.h"
namespace rppicomidi {
class Setup_menu : public Menu
{
public:
    Setup_menu(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor_model& model_, Settings_file& settings_);
    virtual void entry();
private:
    Midi_processor_model& model;
    Save_settings_menu_item save_exit;
    Quit_text_menu_item quit_exit;
    //Int_spinner mc_port;
    static Setup_menu* _instance;
    #if 0
    static int get_mc_port_value();
    static void set_mc_port_value(int value_);
    #endif
};
}