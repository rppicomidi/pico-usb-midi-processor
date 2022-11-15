/**
 * @file restore_view.h
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
#include <string>
#include <vector>
#include "menu.h"
#include "callback_menu_item.h"
#include "text_item_chooser_menu.h"
namespace rppicomidi
{
class Restore_view : public View
{
public:
    Restore_view()=delete;
    virtual ~Restore_view() = default;
    Restore_view(Mono_graphics& screen_);
    void entry() final;
    void exit() final;
    void draw() final;
    Select_result on_select(View** new_view) final {return current_menu->on_select(new_view);}
    void on_increment(uint32_t delta, bool is_shifted) final {current_menu->on_increment(delta, is_shifted); };
    void on_decrement(uint32_t delta, bool is_shifted) final {current_menu->on_decrement(delta, is_shifted); };
private:
    static void dir_select_callback(View* context, View**);
    static void file_select_callback(View* context, View**);
    const Mono_mono_font& font;
    Menu menu;
    Menu dir_chooser_menu;
    Menu* current_menu;
    std::vector<std::string> filenames;
    static constexpr const char* all_files_str="All files";
};
}