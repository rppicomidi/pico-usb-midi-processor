#pragma once
#include "view.h"
namespace rppicomidi
{
class Midi_processing_setup_screen : public View
{
public:
    Midi_processing_setup_screen(Mono_graphics& screen_, const Rectangle& rect_, uint8_t cable_num_, bool is_midi_in_) :
        View{screen_, rect_}, cable_num{cable_num_}, is_midi_in{is_midi_in_} {}
    virtual void draw() { /*TODO*/}
private:
    uint8_t cable_num;
    bool is_midi_in;
};
}