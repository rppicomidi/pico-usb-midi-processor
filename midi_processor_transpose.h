#pragma once
#include "pico/stdlib.h"
#include "midi_processor.h"
#include "setting_number.h"
namespace rppicomidi
{
class Midi_processor_transpose : public Midi_processor
{
public:
    Midi_processor_transpose(uint16_t unique_id) : Midi_processor{static_getname(), unique_id},
        chan{"chan", 1, 16, 1}, min_note{"min_note", 0, 127, 0}, max_note{"max_note", 0, 127, 127},
        transpose_delta("transpose_delta", -12, 12, 0)
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
#if 0

    uint8_t get_chan() {return chan.get(); }
    bool set_note_range(uint8_t min_note_, uint8_t max_note_)
    {
        bool success = false;
        if (min_note.set(min_note_)) {
            max_note.set_min(min_note_);
            success = max_note.set(max_note_);
        }
        dirty = dirty || success;
        return success;
    }

    void get_note_range(uint8_t& min_note_, uint8_t& max_note_)
    {
        min_note_ = min_note.get(); max_note_ = max_note.get();
    }

    bool set_transpose_delta(int8_t delta_)
    {
        bool success = transpose_delta.set(delta_);
        dirty = dirty || success;
        return success;
    }
#endif

    int8_t get_transpose_delta() {return transpose_delta.get(); }
    char* serialize_settings() final;
    bool deserialize_settings(const char* settings_str) final;
    void load_defaults() final;

    // The following are manditory static methods to enable the Midi_processor_manager class
    static const char* static_getname() { return "Transpose"; }
    static Midi_processor* static_make_new(uint16_t unique_id_) {return new Midi_processor_transpose(unique_id_); }
protected:
    Setting_number<uint8_t> chan;           //!< MIDI Channel Number from 1
    Setting_number<uint8_t> min_note;       //!< Minimum note number to transpose 0-127
    Setting_number<uint8_t> max_note;       //!< Maximum note number to transpose min_note-127
    Setting_number<int8_t> transpose_delta; //!< Number of half-steps to add to the note number -12 to 12
};
}