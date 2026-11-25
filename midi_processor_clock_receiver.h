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
class Midi_processor_clock_receiver_settings_view;

class Midi_processor_clock_receiver : public Midi_processor
{
public:
    Midi_processor_clock_receiver(uint16_t unique_id, Midi_buffer* proc_buf_,
        Thread_safe_midi_buffer* in_buf_, Midi_buffer* fb_buf_, Thread_safe_midi_buffer* out_buf_) :
        Midi_processor{static_getname(), unique_id, proc_buf_, in_buf_, fb_buf_, out_buf_},
        bpm{0}, bpm_idx{0}, n_bpm_samples{0}, last_midi_clock_time{0}, view{nullptr} {
            memset(bpm_samples_us, 0, sizeof(bpm));
    }
    virtual ~Midi_processor_clock_receiver()=default;
    bool has_task() final {return true; }

    void process() final {
        now = get_absolute_time();
        size_t npackets = proc_buf->getSize();
        for (size_t idx=0; idx < npackets; idx++) {
            Midi_packet pkt;
            if (!proc_buf->pop(pkt))
                break;
            if (process_one(pkt)) {
                proc_buf->push(pkt);
            }
        }
    }
    /**
     * @brief for each clock packet received, average the arrival time with the previous
     * (max_bpm_samples - 1) samples and compute the new BPM. If there is a new BPM value
     * request redrawing the BPM value
     */
    bool process_one(Midi_packet& pkt) final;

    /**
     * @brief If a new clock packet has not arrived sufficiently recently, request to
     * clear the BPM display
     */
    void task() final;

    /**
     * @return the current BPM value
     */
    uint16_t get_bpm() const {return bpm; }

    /**
     * @brief called by the Midi_processor_clock_receiver_settings_view constructor to
     * give access to the Midi_processor_clock_receiver_settings_view class's public methods.
     */
    void set_view(Midi_processor_clock_receiver_settings_view* view_) {view = view_; }

    // The following are manditory static methods to enable the Midi_processor_manager class
    static const char* static_getname() { return "Clock Receiver"; }
    static Midi_processor* static_make_new(uint16_t unique_id_, Midi_buffer* proc_buf_, Thread_safe_midi_buffer* in_buf_, Midi_buffer* fb_buf_, Thread_safe_midi_buffer* out_buf_) {
        return new Midi_processor_clock_receiver(unique_id_, proc_buf_, in_buf_, fb_buf_, out_buf_);
    }

protected:
    static const size_t max_bpm_samples = 24;
    int64_t bpm_samples_us[max_bpm_samples];
    uint16_t bpm;
    size_t bpm_idx;
    size_t n_bpm_samples;
    absolute_time_t last_midi_clock_time;
    Midi_processor_clock_receiver_settings_view* view;
    absolute_time_t now;
};
}