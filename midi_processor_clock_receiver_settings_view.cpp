/**
 * @file midi_processor_clock_receiver_settings_view.cpp
 * @author rppicomidi
 * @brief BPM display for the clock receiver processor
 *
 * MIT License
 *
 * Copyright (c) 2026 rppicomidi
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
#include <cstdio>
#include "midi_processor_clock_receiver_settings_view.h"
#include "int_spinner_menu_item.h"
rppicomidi::Midi_processor_clock_receiver_settings_view::
    Midi_processor_clock_receiver_settings_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_) :
        Midi_processor_settings_view{screen_, rect_, proc_}, font{screen_.get_font_12()}, is_visible{false}
{
    auto clockrx_proc = reinterpret_cast<Midi_processor_clock_receiver*>(proc);
    clockrx_proc->set_view(this);
}

void rppicomidi::Midi_processor_clock_receiver_settings_view::draw_bpm_from_process(uint16_t bpm)
{
    if (is_visible) {
        // Clear the drawing area
        screen.draw_rectangle(0, font.height, 20 * font.width, font.height*2,
        rppicomidi::Pixel_state::PIXEL_ZERO, rppicomidi::Pixel_state::PIXEL_ZERO);
        draw_bpm(bpm);
    }
}

void rppicomidi::Midi_processor_clock_receiver_settings_view::draw_bpm(uint16_t bpm)
{
    char line[21];
    if (bpm >= 5)
        sprintf(line, "BPM: %-3u", bpm);
    else
        strcpy(line, "BPM: ---");

    screen.draw_string(font, 0, font.height, line, strlen(line));
}

void rppicomidi::Midi_processor_clock_receiver_settings_view::draw()
{
    screen.clear_canvas();
    screen.center_string(font, "Clock Receiver", 0);
    auto clockrx_proc = reinterpret_cast<Midi_processor_clock_receiver*>(proc);
    auto bpm = clockrx_proc->get_bpm();
    draw_bpm(bpm);
}
