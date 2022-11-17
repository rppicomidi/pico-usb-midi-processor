/**
 * @file settings_flash_view.h
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
#include "view.h"
#include "menu.h"
#include "callback_menu_item.h"
namespace rppicomidi
{
class Settings_flash_view : public View
{
public:
    Settings_flash_view()=delete;
    virtual ~Settings_flash_view()=default;
    Settings_flash_view(Mono_graphics& screen_);
    void entry() final;
    void draw() final;
    Select_result on_select(View** new_view) final {return menu.on_select(new_view);}
    void on_increment(uint32_t delta, bool is_shifted) final;
    void on_decrement(uint32_t delta, bool is_shifted) final;
private:
    static void static_delete_all(View*, View**);
    static void static_reformat(View*, View**);
    static void static_delete_file(View*, View**);
    void update_product_string_display();
    Mono_mono_font font;
    Menu menu;
    static const uint8_t max_line_length = 21;
};
}