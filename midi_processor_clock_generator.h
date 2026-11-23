/* MIT License
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
#pragma once
#include "pico/stdlib.h"
#include "pico/time.h"
#include "midi_processor.h"
#include "setting_number.h"
namespace rppicomidi
{
class Midi_processor_clock_generator : public Midi_processor
{
public:
    Midi_processor_clock_generator(uint16_t unique_id, Midi_buffer* proc_buf_, 
        Thread_safe_midi_buffer* in_buf_, Midi_buffer* fb_buf_, Thread_safe_midi_buffer* out_buf_) : 
        Midi_processor{static_getname(), unique_id, proc_buf_, in_buf_, fb_buf_, out_buf_},
        bpm{"bpm", 24, 240, 120}, next_midi_clock_time{0} {
            set_bpm_delta_us(bpm.get());
    }
    virtual ~Midi_processor_clock_generator()=default;
    bool has_task() final {return true; }
    void task() final;
    static uint16_t static_bpm_incr(void* context_, int delta)
    {
        auto me = reinterpret_cast<Midi_processor_clock_generator *>(context_);
        auto current = me->bpm.get();
        auto newval = me->bpm.incr(delta);
        me->dirty = current != newval;
        if (me->dirty) {
            me->set_bpm_delta_us(newval);
        }
        return newval;
    }
    static uint16_t static_bpm_get(void* context_) {
        auto me = reinterpret_cast<Midi_processor_clock_generator *>(context_);
        return  me->bpm.get();
    }
    void serialize_settings(const char* name, JSON_Object *root_object) final;
    bool deserialize_settings(JSON_Object *root_object) final;
    void load_defaults() final;

    // The following are manditory static methods to enable the Midi_processor_manager class
    static const char* static_getname() { return "Clock Generator"; }
    static Midi_processor* static_make_new(uint16_t unique_id_, Midi_buffer* proc_buf_, Thread_safe_midi_buffer* in_buf_, Midi_buffer* fb_buf_, Thread_safe_midi_buffer* out_buf_) {
        return new Midi_processor_clock_generator(unique_id_, proc_buf_, in_buf_, fb_buf_, out_buf_);
    }

    void set_bpm_delta_us(uint16_t bpm_) {
        // 60 seconds in microsecond units divided by beats per minute * 24 clock messages per quarter note
        bpm_delta_us = 60000000ull / (bpm_ * 24);
    }
protected:
    Setting_number<uint16_t> bpm;
    absolute_time_t next_midi_clock_time;
    uint64_t bpm_delta_us;
};
}