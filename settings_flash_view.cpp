/**
 * @file restore_view.cpp
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
#include <vector>
#include <string>
#include "settings_flash_view.h"
#include "callback_menu_item.h"
#include "settings_file.h"
#include "midi_processor_manager.h"
rppicomidi::Settings_flash_view::Settings_flash_view(Mono_graphics& screen_) : View{screen_, screen_.get_clip_rect()}, 
    font{screen.get_font_12()}, menu{screen, static_cast<uint8_t>(2*font.height), font}
{
}

void rppicomidi::Settings_flash_view::entry()
{
    menu.clear();
    auto item = new Callback_menu_item("Delete all", screen, font, this, static_delete_all, Select_result::exit_view);
    menu.add_menu_item(item);
    item = new Callback_menu_item("Reformat memory", screen, font, this, static_reformat, Select_result::exit_view);
    menu.add_menu_item(item);
    std::vector<std::string> filenames;
    Settings_file::instance().get_all_preset_filenames(filenames);
    for (auto& fn : filenames) {
        item = new Callback_menu_item(fn.c_str(), screen, font, this, static_delete_file, Select_result::exit_view);
        menu.add_menu_item(item);
    }
    menu.entry();
}

void rppicomidi::Settings_flash_view::draw()
{
    screen.clear_canvas();
    screen.center_string(font, "Delete Presets", font.height);
    menu.draw();
}

void rppicomidi::Settings_flash_view::update_product_string_display()
{
    // update the device name display at the top of the screen
    char prod_string[max_line_length*2+1];
    if (menu.get_current_item_idx() < 2) {
        screen.draw_rectangle(0, 0, screen.get_screen_width(), 2*font.height,Pixel_state::PIXEL_ZERO, Pixel_state::PIXEL_ZERO);
        screen.center_string(font, "Delete presets", font.height);
    }
    else {
        char* json_format;
        if (Settings_file::instance().load_settings_string(menu.get_current_item()->get_text(), &json_format) > 0) {
            if (!Midi_processor_manager::instance().get_product_string_from_setting_data(json_format, prod_string, sizeof(prod_string))) {
                printf("failed to parse JSON data\r\n");
                delete[] json_format;
                return;
            }
            delete[] json_format;
            screen.center_string_on_two_lines(font, prod_string, 0);
        }
    }
}
void rppicomidi::Settings_flash_view::static_delete_all(View* context, View**)
{
    auto me = reinterpret_cast<Settings_flash_view*>(context);
    int err = Settings_file::instance().delete_all_files("");
    if (err != 0) {
        auto item = reinterpret_cast<Callback_menu_item*>(me->menu.get_current_item());
        item->set_select_action(Select_result::no_op); // do not exit this view.
    }
}

void rppicomidi::Settings_flash_view::static_reformat(View* context, View**)
{
    auto me = reinterpret_cast<Settings_flash_view*>(context);
    int err = pico_mount(true);
    if (err == LFS_ERR_OK) {
        err = pico_unmount();
    }
    if (err != 0) {
        auto item = reinterpret_cast<Callback_menu_item*>(me->menu.get_current_item());
        item->set_select_action(Select_result::no_op); // do not exit this view.
    }
}

void rppicomidi::Settings_flash_view::static_delete_file(View* context, View**)
{
    auto me = reinterpret_cast<Settings_flash_view*>(context);
    int err = Settings_file::instance().delete_file(me->menu.get_current_item()->get_text());
    if (err != 0) {
        auto item = reinterpret_cast<Callback_menu_item*>(me->menu.get_current_item());
        item->set_select_action(Select_result::no_op); // do not exit this view.
    }
}

void rppicomidi::Settings_flash_view::on_increment(uint32_t delta, bool is_shifted)
{
    menu.on_increment(delta, is_shifted);
    update_product_string_display();
}

void rppicomidi::Settings_flash_view::on_decrement(uint32_t delta, bool is_shifted)
{
    menu.on_decrement(delta, is_shifted);
    update_product_string_display();
}
