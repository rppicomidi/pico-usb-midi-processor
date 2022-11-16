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
#include "restore_view.h"
#include "settings_file.h"
#include "midi_processor_manager.h"
rppicomidi::Restore_view::Restore_view(Mono_graphics& screen_) : 
    View{screen_, screen_.get_clip_rect()}, font{screen_.get_font_12()},
        menu{screen, static_cast<uint8_t>(font.height*2), font},
        dir_chooser_menu{screen, static_cast<uint8_t>(font.height*2), font}
{
    current_menu = &menu;
}

void rppicomidi::Restore_view::dir_select_callback(View* context, View**)
{
    auto me = reinterpret_cast<Restore_view*>(context);
    const char* dirname = me->menu.get_current_item()->get_text();
    me->filenames.clear();
    me->filenames.push_back(std::string(all_files_str));
    if (Settings_file::instance().get_all_preset_filenames(dirname, me->filenames)) {
        me->dir_chooser_menu.clear();
        for (auto& filename: me->filenames) {
            auto item = new Callback_menu_item(filename.c_str(), me->screen, me->font, me, file_select_callback);
            item->set_select_action(Select_result::exit_view);
            me->dir_chooser_menu.add_menu_item(item);
        }
    }
    me->current_menu->exit();
    me->current_menu = &me->dir_chooser_menu;
    me->current_menu->entry();
    me->draw();
}

void rppicomidi::Restore_view::file_select_callback(View* context, View**)
{
    auto me = reinterpret_cast<Restore_view*>(context);
    char restore_path[128];
    strcpy(restore_path, me->menu.get_current_item()->get_text());
    int res = strcmp(me->dir_chooser_menu.get_current_item()->get_text(), all_files_str);
    if (res != 0) {
        const char* filename = me->dir_chooser_menu.get_current_item()->get_text();
        strcat(restore_path,"/");
        strcat(restore_path, filename);
    }
    if (Settings_file::instance().restore_presets(restore_path) != 0) {
        printf("error restoring presets from %s", restore_path);
        // clear the top line
        me->screen.draw_rectangle(0, 0, me->screen.get_screen_width(),me->font.height,Pixel_state::PIXEL_ZERO, Pixel_state::PIXEL_ZERO);
        me->screen.center_string(me->font, "Restore Failed", 0);
        reinterpret_cast<Callback_menu_item*>(me->dir_chooser_menu.get_current_item())->set_select_action(Select_result::no_op);
    }
}

void rppicomidi::Restore_view::entry()
{
    current_menu = &menu;
    // rebuild the menu based on the current backup directory list
    menu.clear();
    std::vector<std::string> dirnames;
    if (Settings_file::instance().get_all_preset_directory_names(dirnames)) {
        for (auto& dirname: dirnames) {
            auto item = new Callback_menu_item(dirname.c_str(), screen, font, this, dir_select_callback);
            menu.add_menu_item(item);
        }
        menu.entry();
    }
}

void rppicomidi::Restore_view::exit()
{
    current_menu = &menu;
    menu.exit();
    dir_chooser_menu.exit();
}

void rppicomidi::Restore_view::draw()
{
    screen.clear_canvas();
    screen.center_string(font, "Restore presets", font.height);
    current_menu->draw();
}

void rppicomidi::Restore_view::update_product_string_display()
{
    if (current_menu == &dir_chooser_menu) {
        // update the device name display at the top of the screen
        char prod_string[max_line_length*2+1];
        screen.draw_rectangle(0, 0, screen.get_screen_width(), 2*font.height,Pixel_state::PIXEL_ZERO, Pixel_state::PIXEL_ZERO);
        if (dir_chooser_menu.get_current_item_idx() == 0) {
            screen.center_string(font, "Restore presets", font.height);
        }
        else {
            char* json_format;
            if (Settings_file::instance().get_setting_file_json_string(menu.get_current_item()->get_text(),
                    dir_chooser_menu.get_current_item()->get_text(), &json_format)) {
                if (!Midi_processor_manager::instance().get_product_string_from_setting_data(json_format, prod_string, sizeof(prod_string))) {
                    printf("failed to parse JSON data\r\n");
                    delete[] json_format;
                    return;
                }
                delete[] json_format;
                size_t device_label_len = strlen(prod_string);
                if (device_label_len <= max_line_length) {
                    // Center the Produce String on the 2nd line of the screen
                    screen.center_string(font, prod_string, font.height);
                }
                else {
                    // Break the device_label string into two lines.
                    char line1[max_line_length+1];
                    char line2[max_line_length+1];
                    // Copy as much of the text onto the first line as possible and copy the remaining
                    // text to the next line
                    strncpy(line1, prod_string, max_line_length);
                    line1[max_line_length] = '\0';
                    strncpy(line2, prod_string+max_line_length, max_line_length);
                    line2[max_line_length] = '\0';

                    // See if we can break the text at a space
                    char* ptr = strrchr(line1, ' ');
                    bool center = false;
                    if (ptr != nullptr) {
                        // Found the last space
                        char* remaining_text = prod_string + (ptr - line1 + 1);
                        if (strlen(remaining_text) <= max_line_length) {
                            // Terminate line 1 at the last space
                            *ptr = '\0';
                            // copy the remaining text to line 2
                            strncpy(line2, remaining_text, max_line_length);
                            line2[max_line_length] = '\0';
                            center = true; // center both lines of text for a cleaner look
                        }
                    }
                    if (center) {
                        screen.center_string(font, line1, 0);
                        screen.center_string(font, line2, font.height);
                    }
                    else {
                        screen.draw_string(font, 0, 0, line1, strlen(line1), Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
                        screen.draw_string(font, 0, font.height, line2, strlen(line2), Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
                    }
                }
            }
        }
    }
}

void rppicomidi::Restore_view::on_increment(uint32_t delta, bool is_shifted)
{
    current_menu->on_increment(delta, is_shifted);
    update_product_string_display();
}

void rppicomidi::Restore_view::on_decrement(uint32_t delta, bool is_shifted)
{
    current_menu->on_decrement(delta, is_shifted);
    update_product_string_display();
}