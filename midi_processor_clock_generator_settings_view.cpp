/**
 * @file midi_processor_chan_mes_remap_settings_view.cpp
 * @author rppicomidi
 * @brief Settings UI for channel message remap
 *
 * The UI looks like this:
 *      7-bit or 14-bit
 *      Channel: 1
 *      Display Hex
 *      PB Chan->CC:01->1B
 *      PB Chan->CC:02->2B
 *      Add New PB Chan->CC
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
#include <cassert>
#include <string>
#include "midi_processor_clock_generator_settings_view.h"
#include "int_spinner_menu_item.h"
rppicomidi::Midi_processor_clock_generator_settings_view::
    Midi_processor_clock_generator_settings_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_) :
        Midi_processor_settings_view{screen_, rect_, proc_}, font{screen_.get_font_12()}, menu{screen_, font.height, font}
{
}

void rppicomidi::Midi_processor_clock_generator_settings_view::entry()
{
    auto clockgen_proc = reinterpret_cast<Midi_processor_clock_generator*>(proc);
    menu.clear();

    auto bpm_menu_item = new Int_spinner_menu_item<uint16_t>("BPM:",screen, font, 3, 3, false,
            Midi_processor_clock_generator::static_bpm_get,
            Midi_processor_clock_generator::static_bpm_incr, reinterpret_cast<void*>(clockgen_proc));
    assert(bpm_menu_item);
    menu.add_menu_item(bpm_menu_item);
    menu.entry();
}

void rppicomidi::Midi_processor_clock_generator_settings_view::draw()
{
    screen.clear_canvas();
    screen.center_string(font, "Clock Generator", 0);
    menu.draw();
}
