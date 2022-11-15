/* MIT License
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
#include <cstring>
#include "pico/stdlib.h"

#include "menu.h"
#include "int_spinner_menu_item.h"
#include "midi_processor_settings_view.h"
#include "midi_processor_transpose.h"
#include "callback_menu_item.h"

namespace rppicomidi
{
class Midi_processor_transpose_view : public Midi_processor_settings_view
{
public:
    Midi_processor_transpose_view()=delete;
    virtual ~Midi_processor_transpose_view()=default;
    Midi_processor_transpose_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_);
    void draw() final;

    void entry() final;
    void exit() final {menu.exit();}
    Select_result on_select(View** new_view) final { return menu.on_select(new_view);}
    void on_increment(uint32_t delta, bool is_shifted) final { menu.on_increment(delta, is_shifted); }
    void on_decrement(uint32_t delta, bool is_shifted) final { menu.on_decrement(delta, is_shifted); }
    static void static_toggle_display_format(View* context, View**);
    static Midi_processor_settings_view* static_make_new(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_)
    {
        return new Midi_processor_transpose_view(screen_, rect_, proc_);
    }
private:
    void fix_display_format();
    Menu menu;
    Mono_mono_font font;
    Callback_menu_item* display_format_item;
    Int_spinner_menu_item<uint8_t>* min_note;
    Int_spinner_menu_item<uint8_t>* max_note;
};
}