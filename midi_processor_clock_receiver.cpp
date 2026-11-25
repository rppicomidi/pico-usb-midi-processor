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
#include "midi_processor_clock_receiver.h"
#include "midi_processor_clock_receiver_settings_view.h"
#include "parson.h"
#define MICROSECONDS_PER_MINUTE 60000000ull
#if 0
void rppicomidi::Midi_processor_clock_receiver::serialize_settings(const char* name, JSON_Object *root_object)
{
    (void)name;
    (void)root_object;
    /*
    JSON_Value *proc_value = json_value_init_object();
    JSON_Object *proc_object = json_value_get_object(proc_value);
    bpm.serialize(proc_object);
    json_object_set_value(root_object, name, proc_value);
    */
    dirty = false;
}

bool rppicomidi::Midi_processor_clock_receiver::deserialize_settings(JSON_Object *root_object)
{
    (void)root_object;
    return true;
    /*
    bool result = false;
    if (bpm.deserialize(root_object))
        result = true;

    if (result) {
        dirty = false;
    }
    return result;
    */
}

void rppicomidi::Midi_processor_clock_receiver::load_defaults()
{
    //bpm.set_default();
    dirty = false;
}
#endif

void rppicomidi::Midi_processor_clock_receiver::task()
{
    if (bpm == 0)
        return;
    absolute_time_t now_ = get_absolute_time();
    auto deltat = absolute_time_diff_us(last_midi_clock_time, now_);
    int64_t bpm_tick_us_timeout = MICROSECONDS_PER_MINUTE * 10ull / (24ull * bpm); // reset bpm if (10x bpm)
    if (deltat > bpm_tick_us_timeout) {
        bpm = 0;
        n_bpm_samples = 0;
        if (view != nullptr) {
            view->draw_bpm_from_process(bpm);
        }
    }
}

bool rppicomidi::Midi_processor_clock_receiver::process_one(Midi_packet& pkt)
{
    if (pkt.getStatus() == 0xF8) {
        // MIDI Clock message
        auto deltat = absolute_time_diff_us(last_midi_clock_time, now);
        bpm_samples_us[bpm_idx++] = deltat;
        if (bpm_idx >= max_bpm_samples)
            bpm_idx = 0;
        if (n_bpm_samples < max_bpm_samples) {
            ++n_bpm_samples;
        }
        else {
            // take the average of the last max_bpm_samples samples as the bpm
            uint16_t prev_bpm = bpm;
            bpm = 0;
            int64_t tavg = 0;
            for (size_t idx = 0; idx < max_bpm_samples; idx++) {
                tavg += bpm_samples_us[idx];
            }
            tavg /= max_bpm_samples;
            // compute the BPM; use 0.5 rounding
            bpm = static_cast<uint16_t>((MICROSECONDS_PER_MINUTE * 10ull)/ (24ull * tavg));
            uint16_t inc = 0;
            if ((bpm % 10) >= 5) {
                inc = 1;
            }
            bpm = (bpm / 10) + inc;
            if (view != nullptr && last_midi_clock_time != 0 && bpm != prev_bpm) {
                view->draw_bpm_from_process(bpm);
            }
        }
        last_midi_clock_time = now;
    }
    return true;
}