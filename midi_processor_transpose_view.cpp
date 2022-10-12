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
        menu{screen, screen.get_font_12().height, screen.get_font_12()}
{
    // Make sure that the proc points to a Midi_processor_transpose object (this c++ does not have dynamic cast)
    size_t name_len = strlen(proc->get_name());
    assert(name_len == strlen(Midi_processor_transpose::static_getname()));
    assert(strcmp(proc->get_name(),Midi_processor_transpose::static_getname()) == 0);
    auto chan = new Int_spinner_menu_item<uint8_t>("MIDI chan: ",screen,screen.get_font_12(),3, 3,false,Midi_processor_transpose::static_get_chan, Midi_processor_transpose::static_incr_chan, proc_);
    assert(chan);
    auto min_note = new Int_spinner_menu_item<uint8_t>("Min MIDI note: ", screen, screen.get_font_12(), 3, 2, false, Midi_processor_transpose::static_get_min_note, Midi_processor_transpose::static_incr_min_note, proc_);
    assert(min_note);
    auto max_note = new Int_spinner_menu_item<uint8_t>("Max MIDI note: ", screen, screen.get_font_12(), 3, 2, false, Midi_processor_transpose::static_get_max_note, Midi_processor_transpose::static_incr_max_note, proc_);
    assert(max_note);
    auto transpose_delta = new Int_spinner_menu_item<int8_t>("Halfstep delta: ", screen, screen.get_font_12(), 3, 3, false, Midi_processor_transpose::static_get_transpose_delta, Midi_processor_transpose::static_incr_transpose_delta, proc_);
    assert(transpose_delta);
    menu.add_menu_item(chan);
    menu.add_menu_item(min_note);
    menu.add_menu_item(max_note);
    menu.add_menu_item(transpose_delta);
}

void rppicomidi::Midi_processor_transpose_view::draw()
{
    screen.clear_canvas();
    screen.center_string(screen.get_font_12(), "Transpose Settings", 0);
    menu.draw();
}