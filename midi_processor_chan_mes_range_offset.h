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
#include <map>
#include "midi_processor.h"
#include "setting_number.h"
#include "setting_string_enum.h"
#include "setting_bimap.h"
#include "pico/mutex.h"
namespace rppicomidi
{
/**
 * @brief This class implements a generalized mapper that remaps any
 * channel message other than pitch bend to any other. It can also
 * re-channelize messages over ranges of values, and it can add or
 * subtract a fixed offset from data bytes. It is useful for
 * creating keyboard splits, mapping CC messages to channel
 * pressure messages, and other specialized applications.
 */
class Midi_processor_chan_mes_range_offset : public Midi_processor
{
public:
    Midi_processor_chan_mes_range_offset(const char* name_, uint16_t unique_id);
    Midi_processor_chan_mes_range_offset(uint16_t unique_id) : Midi_processor_chan_mes_range_offset(static_getname(), unique_id) {}
    Midi_processor_chan_mes_range_offset() = delete;
    virtual ~Midi_processor_chan_mes_range_offset()=default;

    bool process(uint8_t *packet) final;
    virtual bool has_feedback_process() {return false; }
    const std::vector<std::string>* get_all_possible_channel_message_types() const { return message_type_in.get_all_possible_values(); }
    bool set_message_in_type(size_t idx) { dirty = message_type_in.get_ivalue() != (int)idx; return message_type_in.set(idx); }
    void get_message_in_type(std::string &typestr) { message_type_in.get(typestr); }
    int get_message_in_idx() {return message_type_in.get_ivalue(); }
    bool set_message_out_type(size_t idx) { dirty = message_type_out.get_ivalue() != (int)idx; return message_type_out.set(idx); }
    void get_message_out_type(std::string &typestr) { message_type_out.get(typestr); }
    int get_message_out_idx() {return message_type_out.get_ivalue(); }
    bool set_display_format(size_t idx) { dirty = true; return display_format.set(idx); }
    void get_display_format(std::string &typestr) { display_format.get(typestr); }
    size_t get_display_format() {return display_format.get_ivalue(); }
    const std::vector<std::string>* get_all_possible_display_formats() const { return display_format.get_all_possible_values(); }
    void serialize_settings(const char* name, JSON_Object *root_object) final;
    bool deserialize_settings(JSON_Object *root_object) final;

    static uint8_t static_get_byte1_range(void *context_, size_t idx, size_t element_idx_)
    {
        if (idx != 0)
            return 0xff;

        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        return me->byte1_range.get(idx, element_idx_);
    }

    static uint8_t static_incr_byte1_range(void *context_, size_t idx, size_t element_idx_, int delta)
    {
        if (idx != 0)
            return 0xff;

        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        return me->byte1_range.incr(idx, element_idx_, delta);
    }

    static uint8_t static_get_byte2_range(void *context_, size_t idx, size_t element_idx_)
    {
        if (idx != 0)
            return 0xff;

        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        return me->byte2_range.get(idx, element_idx_);
    }

    static uint8_t static_incr_byte2_range(void *context_, size_t idx, size_t element_idx_, int delta)
    {
        if (idx != 0)
            return 0xff;

        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        return me->byte2_range.incr(idx, element_idx_, delta);
    }

    static int8_t static_get_byte1_offset(void *context_)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        return me->byte1_offset.get();
    }

    static int8_t static_get_byte2_offset(void *context_)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        return me->byte2_offset.get();
    }

    static int8_t static_incr_byte1_offset(void *context_, int delta_)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        auto current = me->byte1_offset.get();
        auto newval = me->byte1_offset.incr(delta_);
        me->dirty = current != newval;

        return newval;
    }

    static int8_t static_incr_byte2_offset(void *context_, int delta_)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        auto current = me->byte2_offset.get();
        auto newval = me->byte2_offset.incr(delta_);
        me->dirty = current != newval;

        return newval;
    }

    static uint8_t static_chan_in_get(void* context_)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        return me->chan_in.get();
    }

    static uint8_t static_chan_out_get(void* context_)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        return me->chan_out.get();
    }

    static uint8_t static_chan_in_incr(void* context_, int delta)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        auto current = me->chan_in.get();
        auto newval = me->chan_in.incr(delta);
        me->dirty = current != newval;

        return newval;
    }

    static uint8_t static_chan_out_incr(void* context_, int delta)
    {
        auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset *>(context_);
        auto current = me->chan_out.get();
        auto newval = me->chan_out.incr(delta);
        me->dirty = current != newval;

        return newval;
    }

    static const char *static_getname() { return "Chan Mes Range Offset"; }
    static Midi_processor* static_make_new(uint16_t unique_id_) { return new Midi_processor_chan_mes_range_offset(unique_id_); }
protected:
    bool process_internal(uint8_t *packet, size_t first_idx, size_t second_idx);
    const std::string note_off_msg;
    const std::string note_on_msg;
    const std::string cc_msg;
    const std::string poly_pressure_msg;
    const std::string chan_pressure_msg;
    const std::string prog_change_msg;
    const std::string format_decimal;
    const std::string format_hex;
    Setting_number<uint8_t> chan_in;
    Setting_number<uint8_t> chan_out;
    Setting_string_enum message_type_in;
    Setting_string_enum message_type_out;
    Setting_bimap<uint8_t> byte1_range;
    Setting_bimap<uint8_t> byte2_range;
    Setting_number<int8_t> byte1_offset;
    Setting_number<int8_t> byte2_offset;
    Setting_string_enum display_format;
    mutex processing_mutex;
};
}