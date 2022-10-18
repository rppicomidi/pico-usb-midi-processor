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
namespace rppicomidi
{
class Midi_processor_transpose : public Midi_processor
{
public:
    Midi_processor_transpose(uint16_t unique_id) : Midi_processor{static_getname(), unique_id},
        chan{"chan", 1, 16, 1}, min_note{"min_note", 0, 127, 0}, max_note{"max_note", 0, 127, 127},
        transpose_delta("transpose_delta", -12, 12, 0),
        format_decimal{"Display Decimal"}, format_hex{"Display Hex"},
        display_format{"Display Format", {format_decimal, format_hex}}

    {
        load_defaults();
    }
    virtual ~Midi_processor_transpose()=default;
    bool process(uint8_t* packet) final;
    static uint8_t static_get_chan(void* context)
    {
        auto me = reinterpret_cast<Midi_processor_transpose*>(context);
        return me->chan.get();
    }
    static uint8_t static_incr_chan(void* context, int delta)
    {
        auto me = reinterpret_cast<Midi_processor_transpose*>(context);
        uint8_t oldval = me->chan.get();
        uint8_t newval = me->chan.incr(delta);
        me->dirty = (oldval != newval);
        return newval;
    }

    static uint8_t static_get_min_note(void* context)
    {
        auto me = reinterpret_cast<Midi_processor_transpose*>(context);
        return me->min_note.get();
    }

    static uint8_t static_incr_min_note(void* context, int delta)
    {
        auto me = reinterpret_cast<Midi_processor_transpose*>(context);
        uint8_t oldval = me->min_note.get();
        uint8_t max_note_val = me->max_note.get();
        if ((int)max_note_val < (int)oldval+delta)
            return oldval; // you can't increment the min past the max
        uint8_t newval = me->min_note.incr(delta);
        me->max_note.set_min(newval); // you can't decrement the max note below the min value
        me->dirty = oldval != newval;
        return newval;
    }

    static uint8_t static_get_max_note(void* context)
    {
        auto me = reinterpret_cast<Midi_processor_transpose*>(context);
        return me->max_note.get();
    }

    static uint8_t static_incr_max_note(void* context, int delta)
    {
        auto me = reinterpret_cast<Midi_processor_transpose*>(context);
        uint8_t oldval = me->max_note.get();
        uint8_t newval = me->max_note.incr(delta);
        me->dirty = oldval != newval;
        return newval;
    }

    static int8_t static_get_transpose_delta(void* context)
    {
        auto me = reinterpret_cast<Midi_processor_transpose*>(context);
        return me->transpose_delta.get();
    }

    static int8_t static_incr_transpose_delta(void* context, int delta)
    {
        auto me = reinterpret_cast<Midi_processor_transpose*>(context);
        int8_t oldval = me->transpose_delta.get();
        int8_t newval = me->transpose_delta.incr(delta);
        me->dirty = oldval != newval;
        return newval;
    }

    int8_t get_transpose_delta() {return transpose_delta.get(); }
    void serialize_settings(const char* name, JSON_Object *root_object) final;
    bool deserialize_settings(JSON_Object *root_object) final;
    void load_defaults() final;

    bool set_display_format(size_t idx) { dirty = true; return display_format.set(idx); }
    void get_display_format(std::string &typestr) { display_format.get(typestr); }
    size_t get_display_format() {return display_format.get_ivalue(); }

    // The following are manditory static methods to enable the Midi_processor_manager class
    static const char* static_getname() { return "Transpose"; }
    static Midi_processor* static_make_new(uint16_t unique_id_) {return new Midi_processor_transpose(unique_id_); }
protected:
    Setting_number<uint8_t> chan;           //!< MIDI Channel Number from 1
    Setting_number<uint8_t> min_note;       //!< Minimum note number to transpose 0-127
    Setting_number<uint8_t> max_note;       //!< Maximum note number to transpose min_note-127
    Setting_number<int8_t> transpose_delta; //!< Number of half-steps to add to the note number -12 to 12
    const std::string format_decimal;
    const std::string format_hex;
    Setting_string_enum display_format;     //!< Decimal or hex
};
}