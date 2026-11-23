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
#include <map>
#include "midi_processor.h"
#include "setting_number.h"
#include "setting_string_enum.h"
#include "setting_bimap.h"
#include "pico/mutex.h"
namespace rppicomidi
{
/**
 * @brief This class remaps the byte following the status byte to a new value.
 * If the new value is 128, then the message is filtered out.
 * 
 * This processor is most useful for mapping CC number or Note numbers
 * to new values for button presses. If the button has an LED with illumnination
 * controlled by a return message of the same type and CC number or Note number,
 * use Midi_processor_chan_button_remap class instead; it has a feedback method
 * implemented.
 */
class Midi_processor_pitchbend_to_cc: public Midi_processor
{
public:
    Midi_processor_pitchbend_to_cc(uint16_t unique_id, Midi_buffer* proc_buf_, Thread_safe_midi_buffer* in_buf_, 
        Midi_buffer* fb_buf_, Thread_safe_midi_buffer* out_buf_);
    Midi_processor_pitchbend_to_cc() = delete;
    virtual ~Midi_processor_pitchbend_to_cc()=default;
    int add_remap()
    {
        size_t result = 1;
        mutex_enter_blocking(&processing_mutex);
        if (bimap.size() == 0) {
            result = bimap.push_back(bimap.get_min(0), bimap.get_min(1));
        }
        else if (bimap.size() == 16) {
            mutex_exit(&processing_mutex);
            return -1; // cannot add the remap
        }
        else {
            // find the lowest PB channel that has not been allocated and add that one
            uint8_t chan = 1;
            for (; chan <= 16 && bimap.find(chan, 0) != -1; chan++) {
            }
            if (chan <= 16) {
                result = bimap.push_back(chan, bimap.get_min());
            }
        }
        mutex_exit(&processing_mutex);
        return result;
    }
    void delete_remap(size_t idx)
    {
        mutex_enter_blocking(&processing_mutex);
        bimap.erase(idx);
        mutex_exit(&processing_mutex);
    }
    bool process_one(Midi_packet& pkt);

    virtual bool has_feedback_process() {return false; }
    
    bool set_display_format(size_t idx) { dirty = true; return display_format.set(idx); }
    void get_display_format(std::string &typestr) { display_format.get(typestr); }
    size_t get_display_format() {return display_format.get_ivalue(); }
    const std::vector<std::string>* get_all_possible_display_formats() const { return display_format.get_all_possible_values(); }
    bool set_resolution(size_t idx) { dirty = true; return resolution.set(idx); }
    void get_resolution(std::string &typestr) { resolution.get(typestr); }
    size_t get_resolution() {return resolution.get_ivalue(); }
    const std::vector<std::string>* get_all_possible_resolutions() const { return resolution.get_all_possible_values(); }

    void serialize_settings(const char* name, JSON_Object *root_object) final;
    bool deserialize_settings(JSON_Object *root_object) final;
    size_t get_num_remap() { return bimap.size(); }
    static uint8_t static_get(void *context_, size_t bimap_idx_, size_t element_idx_)
    {
        auto me = reinterpret_cast<Midi_processor_pitchbend_to_cc *>(context_);
        return me->bimap.get(bimap_idx_, element_idx_);
    }
    static uint8_t static_incr(void *context_, size_t bimap_idx_, size_t element_idx_, int delta_)
    {
        auto me = reinterpret_cast<Midi_processor_pitchbend_to_cc *>(context_);
        auto current = me->bimap.get(bimap_idx_, element_idx_);
        auto newval = me->bimap.incr(bimap_idx_, element_idx_, delta_);
        if (newval >= me->bimap.get_max(element_idx_)) {
            newval = me->bimap.get_max(element_idx_) - 1;
            me->bimap.set(bimap_idx_, element_idx_, newval);
        }
        me->dirty = current != newval;

        return newval;
    }
    static uint8_t static_chan_get(void* context_)
    {
        auto me = reinterpret_cast<Midi_processor_pitchbend_to_cc *>(context_);
        return me->chan.get();
    }
    static uint8_t static_chan_incr(void* context_, int delta)
    {
        auto me = reinterpret_cast<Midi_processor_pitchbend_to_cc *>(context_);
        auto current = me->chan.get();
        auto newval = me->chan.incr(delta);
        me->dirty = current != newval;

        return newval;
    }
    static const char *static_getname() { return "Pitchbend to CC"; }
    static Midi_processor* static_make_new(uint16_t unique_id_, Midi_buffer* proc_buf_, Thread_safe_midi_buffer* in_buf_, Midi_buffer* fb_buf_, Thread_safe_midi_buffer* out_buf_) {
        return new Midi_processor_pitchbend_to_cc(unique_id_, proc_buf_, in_buf_, fb_buf_, out_buf_);
    }
protected:
    bool process_internal(Midi_packet&pkt, size_t first_idx, size_t second_idx);

    const std::string format_decimal;
    const std::string format_hex;
    const std::string resolution7;
    const std::string resolution14;
    Setting_string_enum resolution;
    Setting_bimap<uint8_t> bimap; // Pitchbend channel, base CC number; 14-bit resolution LSB byte sent base CC number + 32
    Setting_string_enum display_format;
    Setting_number<uint8_t> chan;
    mutex processing_mutex;
};
}