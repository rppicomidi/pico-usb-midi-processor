/**
 * @file backup_view.h
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
#include "callback_menu_item.h"
namespace rppicomidi {
class Backup_view : public View
{
public:
    Backup_view(Mono_graphics& screen_);

    Backup_view() = delete;

    virtual ~Backup_view()=default;

    void entry() final;
    void draw() final;
    Select_result on_select(View** new_view) final {return menu.on_select(new_view);}
    void on_increment(uint32_t delta, bool is_shifted) final {menu.on_increment(delta, is_shifted); };
    void on_decrement(uint32_t delta, bool is_shifted) final {menu.on_decrement(delta, is_shifted); };
private:
    static void static_start_backup(View* view_);
    const Mono_mono_font& font;
    Menu menu;
    Callback_menu_item* backup_all;
};
}
