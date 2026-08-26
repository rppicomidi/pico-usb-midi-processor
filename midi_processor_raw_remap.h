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
#include "pico/stdlib.h"
#include "midi_processor.h"
#include "setting_number.h"
#include "setting_string_enum.h"
#include "setting_byte3.h"
namespace rppicomidi
{
class Midi_processor_raw_remap : public Midi_processor
{
public:
    Midi_processor_raw_remap(uint16_t unique_id) : Midi_processor{static_getname(), unique_id},
        in_mes{" IN:", {0x80,0x0,0x0}, {0xBF, 0x7f, 0x7f}},
        out_mes{"OUT:", {0x80,0x0,0x0}, {0xBF, 0x7f, 0x7f}},
        format_decimal{"Display Decimal"}, format_hex{"Display Hex"},
        display_format{"Display Format", {format_decimal, format_hex}}

    {
        load_defaults();
    }
    virtual ~Midi_processor_raw_remap()=default;
    bool process(uint8_t* packet) final;
    static uint8_t get_in_mes_byte(void* context, size_t idx) {
        auto me = reinterpret_cast<Midi_processor_raw_remap*>(context);
        return me->in_mes.get(idx);
    }
    static uint8_t get_in_mes_max_byte(void* context, size_t idx) {
        auto me = reinterpret_cast<Midi_processor_raw_remap*>(context);
        return me->in_mes.get_max(idx);
    }

    static uint8_t incr_in_mes_byte(void* context, size_t idx, int delta) {
        auto me = reinterpret_cast<Midi_processor_raw_remap*>(context);
        auto oldval = me->in_mes.get(idx);
        auto newval = me->in_mes.incr(idx, delta);
        me->dirty = oldval != newval;
        return newval;
    }

    static uint8_t get_out_mes_byte(void* context, size_t idx) {
        auto me = reinterpret_cast<Midi_processor_raw_remap*>(context);
        return me->out_mes.get(idx);
    }
    static uint8_t get_out_mes_max_byte(void* context, size_t idx) {
        auto me = reinterpret_cast<Midi_processor_raw_remap*>(context);
        return me->out_mes.get_max(idx);
    }
    static uint8_t incr_out_mes_byte(void* context, size_t idx, int delta) {
        auto me = reinterpret_cast<Midi_processor_raw_remap*>(context);
        auto oldval = me->out_mes.get(idx);
        auto newval = me->out_mes.incr(idx, delta);
        me->dirty = oldval != newval;
        return newval;
    }

    void serialize_settings(const char* name, JSON_Object *root_object) final;
    bool deserialize_settings(JSON_Object *root_object) final;
    void load_defaults() final;

    bool set_display_format(size_t idx) { dirty = true; return display_format.set(idx); }
    void get_display_format(std::string &typestr) { display_format.get(typestr); }
    size_t get_display_format() {return display_format.get_ivalue(); }

    // The following are manditory static methods to enable the Midi_processor_manager class
    static const char* static_getname() { return "Raw Message Remap"; }
    static Midi_processor* static_make_new(uint16_t unique_id_) {return new Midi_processor_raw_remap(unique_id_); }
protected:
    Setting_byte3 in_mes;               //!< raw input message bytes
    Setting_byte3 out_mes;              //!< raw output message bytes
    const std::string format_decimal;
    const std::string format_hex;
    Setting_string_enum display_format;     //!< Decimal or hex
};
}