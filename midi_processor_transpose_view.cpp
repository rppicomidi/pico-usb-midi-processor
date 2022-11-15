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
#include <cstring>
#include "midi_processor_transpose_view.h"
rppicomidi::Midi_processor_transpose_view::Midi_processor_transpose_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_) :
        Midi_processor_settings_view{screen_, rect_, proc_},
        menu{screen, screen.get_font_12().height, screen.get_font_12()},font{screen.get_font_12()}
{
    // Make sure that the proc points to a Midi_processor_transpose object (this c++ does not have dynamic cast)
    size_t name_len = strlen(proc->get_name());
    assert(name_len == strlen(Midi_processor_transpose::static_getname()));
    assert(strcmp(proc->get_name(),Midi_processor_transpose::static_getname()) == 0);
    auto transpose_proc = reinterpret_cast<Midi_processor_transpose*>(proc_);
    auto chan = new Int_spinner_menu_item<uint8_t>("MIDI chan: ",screen, font ,3, 3,false,Midi_processor_transpose::static_get_chan, Midi_processor_transpose::static_incr_chan, proc_);
    assert(chan);
    std::string display_format_str;
    transpose_proc->get_display_format(display_format_str);
    display_format_item = new Callback_menu_item{display_format_str.c_str(), screen, font, this, static_toggle_display_format};
    assert(display_format_item);
    min_note = new Int_spinner_menu_item<uint8_t>("Min MIDI note: ", screen, font, 3, 2, false, Midi_processor_transpose::static_get_min_note, Midi_processor_transpose::static_incr_min_note, proc_);
    assert(min_note);
    max_note = new Int_spinner_menu_item<uint8_t>("Max MIDI note: ", screen, font, 3, 2, false, Midi_processor_transpose::static_get_max_note, Midi_processor_transpose::static_incr_max_note, proc_);
    assert(max_note);
    auto transpose_delta = new Int_spinner_menu_item<int8_t>("Halfstep delta: ", screen, font, 3, 3, false, Midi_processor_transpose::static_get_transpose_delta, Midi_processor_transpose::static_incr_transpose_delta, proc_);
    assert(transpose_delta);
    menu.add_menu_item(chan);
    menu.add_menu_item(transpose_delta);
    menu.add_menu_item(display_format_item);
    menu.add_menu_item(min_note);
    menu.add_menu_item(max_note);
}

void rppicomidi::Midi_processor_transpose_view::draw()
{
    screen.clear_canvas();
    screen.center_string(screen.get_font_12(), "Transpose Settings", 0);
    menu.draw();
}

void rppicomidi::Midi_processor_transpose_view::fix_display_format()
{
    auto transpose_proc = reinterpret_cast<Midi_processor_transpose*>(proc);
    size_t fmt_idx = transpose_proc->get_display_format();
    bool is_hex = (fmt_idx != 0);
    // update the display format menu item string
    std::string fmt_str;
    transpose_proc->get_display_format(fmt_str);
    display_format_item->set_text(fmt_str.c_str());
    min_note->set_display_hex(is_hex);
    max_note->set_display_hex(is_hex);
}

void rppicomidi::Midi_processor_transpose_view::entry()
{
    fix_display_format();
    menu.entry();
}

void rppicomidi::Midi_processor_transpose_view::static_toggle_display_format(View* context, View**)
{
    auto me=reinterpret_cast<Midi_processor_transpose_view*>(context);
    auto transpose_proc = reinterpret_cast<Midi_processor_transpose*>(me->proc);
    size_t fmt_idx = transpose_proc->get_display_format();
    bool is_hex = (fmt_idx == 0); // toggle the display format value
    transpose_proc->set_display_format(is_hex ? 1:0); // update the setting
    me->fix_display_format();
    me->draw();
}
