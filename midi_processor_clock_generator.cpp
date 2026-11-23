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
#include "midi_processor_clock_generator.h"
#include "parson.h"

void rppicomidi::Midi_processor_clock_generator::serialize_settings(const char* name, JSON_Object *root_object)
{
    JSON_Value *proc_value = json_value_init_object();
    JSON_Object *proc_object = json_value_get_object(proc_value);
    bpm.serialize(proc_object);
    json_object_set_value(root_object, name, proc_value);
    dirty = false;
}

bool rppicomidi::Midi_processor_clock_generator::deserialize_settings(JSON_Object *root_object)
{
    bool result = false;
    if (bpm.deserialize(root_object))
        result = true;

    if (result) {
        dirty = false;
    }
    return result;
}

void rppicomidi::Midi_processor_clock_generator::load_defaults()
{
    bpm.set_default();
    dirty = false;
}

void rppicomidi::Midi_processor_clock_generator::task()
{
    uint64_t now = get_absolute_time();
    if (now >= next_midi_clock_time) {
        Midi_packet pkt;
        pkt.setCin(0xF);
        pkt.setStatus(0xF8); // Timing clock message
        this->in_buf->push(pkt);
        // adding the delta to the previous clock time instead of the current
        // clock time should reduce clock jitter
        next_midi_clock_time = delayed_by_us(next_midi_clock_time, bpm_delta_us);
    }
}