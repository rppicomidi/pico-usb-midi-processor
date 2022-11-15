/**
 * @file preset_view.h
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
#include "view.h"
#include "menu.h"
#include "callback_menu_item.h"
#include "view_launch_menu_item.h"
#include "setting_number.h"
#include "int_spinner_menu_item.h"
#include "midi_processor_manager.h"
#pragma once
namespace rppicomidi
{
class Preset_view : public View
{
public:
    Preset_view()=delete;
    Preset_view(Mono_graphics& screen_, const Rectangle& rect_) : 
    View{screen_, rect_}, menu{screen_, screen_.get_font_12().height, screen_.get_font_12()} , next_preset{"next preset",1, 8, 1}
    {
        auto preset_item = new Int_spinner_menu_item<uint8_t>("Next Preset:", screen, screen.get_font_12(), 1,1,false,
            static_get_next_preset, static_incr_next_preset, this);
        menu.add_menu_item(preset_item);
        auto item = new Callback_menu_item("Save next preset",screen, screen.get_font_12(), this,static_save_callback, View::Select_result::exit_view);
        menu.add_menu_item(item);
        item = new Callback_menu_item("Load next preset",screen, screen.get_font_12(), this, static_load_callback, View::Select_result::exit_view);
        menu.add_menu_item(item);
        item = new Callback_menu_item("Reset next preset",screen, screen.get_font_12(), this, static_reset_callback, View::Select_result::exit_view);
        menu.add_menu_item(item);
    }
    void entry() final {next_preset.set(Midi_processor_manager::instance().get_current_preset()); menu.entry(); }
    void draw() final;
    Select_result on_select(View** new_view) final { return menu.on_select(new_view); }
    void on_increment(uint32_t delta, bool is_shifted) final {menu.on_increment(delta, is_shifted); };
    void on_decrement(uint32_t delta, bool is_shifted) final {menu.on_decrement(delta, is_shifted); };
private:
    static uint8_t static_get_next_preset(void* context);
    static uint8_t static_incr_next_preset(void* context, int delta);
    static void static_save_callback(View*, View**);
    static void static_load_callback(View*, View**);
    static void static_reset_callback(View*, View**);
    Menu menu;
    Setting_number<uint8_t> next_preset;
};
}