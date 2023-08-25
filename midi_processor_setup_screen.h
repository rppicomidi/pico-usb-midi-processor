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
#include <vector>
#include "view.h"
#include "menu.h"
#include "view_launch_menu_item.h"
#include "text_item_chooser_menu.h"
namespace rppicomidi
{
class Midi_processor_setup_screen : public View
{
public:
    Midi_processor_setup_screen(Mono_graphics& screen_, const Mono_mono_font& font_,
        uint8_t cable_num_, bool is_midi_in_);
    void draw() final;
    static void select_callback(View* me, int idx);
    void entry() final;
    void exit() final;
    Select_result on_select(View** new_view) final;
    void on_left(uint32_t delta, bool is_shifted) final; // delete the selected element
    void on_increment(uint32_t delta, bool is_shifted) final {menu.on_increment(delta, is_shifted); };
    void on_decrement(uint32_t delta, bool is_shifted) final {menu.on_decrement(delta, is_shifted); };
private:
    Mono_mono_font font;
    uint8_t cable_num;
    bool is_midi_in;
    Menu menu;
    Text_item_chooser_menu processor_select;
    std::vector<const char*> processor_names;
};
}