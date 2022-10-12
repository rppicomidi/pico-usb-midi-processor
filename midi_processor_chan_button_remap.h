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
#include "midi_processor_chan_mes_remap.h"
namespace rppicomidi {
class Midi_processor_chan_button_remap : public Midi_processor_chan_mes_remap
{
public:
    Midi_processor_chan_button_remap(uint16_t unique_id) : Midi_processor_chan_mes_remap(static_getname(), unique_id)
    {
    }
    Midi_processor_chan_button_remap() = delete;
    virtual ~Midi_processor_chan_button_remap()=default;
    bool feedback(uint8_t *packet) final { return process_internal(packet, 1, 0); }
    bool has_feedback_process() final {return true; }
    static const char *static_getname() { return "Channel Button Remap"; }
    static Midi_processor* static_make_new(uint16_t unique_id_) { return new Midi_processor_chan_button_remap(unique_id_); }

};
}