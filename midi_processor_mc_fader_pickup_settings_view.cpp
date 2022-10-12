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
#include "midi_processor_mc_fader_pickup_settings_view.h"
rppicomidi::Midi_processor_mc_fader_pickup_settings_view::Midi_processor_mc_fader_pickup_settings_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_) :
        Midi_processor_settings_view(screen_, rect_, proc_)
{
    // Make sure that the proc points to a Midi_processor_transpose object (this c++ does not have dynamic cast)
    size_t name_len = strlen(proc->get_name());
    assert(name_len == strlen(Midi_processor_mc_fader_pickup::static_getname()));
    assert(strcmp(proc->get_name(),Midi_processor_mc_fader_pickup::static_getname()) == 0);
    mc_fader_pickup = reinterpret_cast<Midi_processor_mc_fader_pickup*>(proc);
}

void rppicomidi::Midi_processor_mc_fader_pickup_settings_view::draw()
{
    screen.clear_canvas();
    screen.center_string(screen.get_font_12(), "MC Fader Pickup", 0);
    const char* msg= "Nothing to set up";
    screen.draw_string(screen.get_font_12(), 0, screen.get_font_12().height*2, msg, strlen(msg), Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
}