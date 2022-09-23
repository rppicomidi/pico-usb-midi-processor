#pragma once
#include "midi_processor.h"
namespace rppicomidi
{
class Midi_processor_transpose : public Midi_processor
{
public:
    Midi_processor_transpose(uint16_t unique_id) : Midi_processor{static_getname(), unique_id},
        chan{0}, min_note{0}, max_note{127}, transpose_delta{0} {}

    bool process(uint8_t packet[4]) final;
    void set_chan(uint8_t chan_) { if (chan < 0x10) chan= chan_; }
    uint8_t get_chan() {return chan; }
    void set_note_range(uint8_t min_note_, uint8_t max_note_) {min_note = min_note_; max_note = max_note_;}
    void get_note_range(uint8_t& min_note_, uint8_t& max_note_) {min_note_ = min_note; max_note_ = max_note;}
    void set_transpose_delta(int8_t delta_) {transpose_delta = delta_; }
    int8_t get_transpose_delta() {return transpose_delta; }
    static const char* static_getname() { return "Transpose"; }
    static Midi_processor* static_make_new(uint16_t unique_id_) {return new Midi_processor_transpose(unique_id_); }
protected:
    uint8_t chan;
    uint8_t min_note;
    uint8_t max_note;
    int8_t transpose_delta;
};
}