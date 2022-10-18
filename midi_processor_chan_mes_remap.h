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
class Midi_processor_chan_mes_remap : public Midi_processor
{
public:
    Midi_processor_chan_mes_remap(const char* name_, uint16_t unique_id);
    Midi_processor_chan_mes_remap(uint16_t unique_id) : Midi_processor_chan_mes_remap(static_getname(), unique_id) {}
    Midi_processor_chan_mes_remap() = delete;
    virtual ~Midi_processor_chan_mes_remap()=default;
    size_t add_remap()
    {
        mutex_enter_blocking(&processing_mutex);
        auto result = bimap.push_back(bimap.get_max(), bimap.get_max());
        mutex_exit(&processing_mutex);
        return result;
    }
    void delete_remap(size_t idx)
    {
        mutex_enter_blocking(&processing_mutex);
        bimap.erase(idx);
        mutex_exit(&processing_mutex);
    }
    bool process(uint8_t *packet) final { return process_internal(packet, 0, 1); }
    virtual bool has_feedback_process() {return false; }
    const std::vector<std::string>* get_all_possible_channel_message_types() const { return message_type.get_all_possible_values(); }
    bool set_message_type(size_t idx) { dirty = message_type.get_ivalue() != (int)idx; return message_type.set(idx); }
    void get_message_type(std::string &typestr) { message_type.get(typestr); }
    bool set_display_format(size_t idx) { dirty = true; return display_format.set(idx); }
    void get_display_format(std::string &typestr) { display_format.get(typestr); }
    size_t get_display_format() {return display_format.get_ivalue(); }
    const std::vector<std::string>* get_all_possible_display_formats() const { return display_format.get_all_possible_values(); }
    void serialize_settings(const char* name, JSON_Object *root_object) final;
    bool deserialize_settings(JSON_Object *root_object) final;
    size_t get_num_remap() { return bimap.size(); }
    static uint8_t static_get(void *context_, size_t bimap_idx_, size_t element_idx_)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_remap *>(context_);
        return me->bimap.get(bimap_idx_, element_idx_);
    }
    static uint8_t static_incr(void *context_, size_t bimap_idx_, size_t element_idx_, int delta_)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_remap *>(context_);
        auto current = me->bimap.get(bimap_idx_, element_idx_);
        auto newval = me->bimap.incr(bimap_idx_, element_idx_, delta_);
        me->dirty = current != newval;

        return newval;
    }
    static uint8_t static_chan_get(void* context_)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_remap *>(context_);
        return me->chan.get();
    }
    static uint8_t static_chan_incr(void* context_, int delta)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_remap *>(context_);
        auto current = me->chan.get();
        auto newval = me->chan.incr(delta);
        me->dirty = current != newval;

        return newval;
    }
    static const char *static_getname() { return "Channel Message Remap"; }
    static Midi_processor* static_make_new(uint16_t unique_id_) { return new Midi_processor_chan_mes_remap(unique_id_); }
protected:
    bool process_internal(uint8_t *packet, size_t first_idx, size_t second_idx);
    const std::string note_msg;
    const std::string cc_msg;
    const std::string poly_pressure_msg;
    const std::string chan_pressure_msg;
    const std::string prog_change_msg;
    const std::string format_decimal;
    const std::string format_hex;
    Setting_number<uint8_t> chan;
    Setting_string_enum message_type;
    Setting_bimap<uint8_t> bimap;
    Setting_string_enum display_format;
    mutex processing_mutex;
};
}