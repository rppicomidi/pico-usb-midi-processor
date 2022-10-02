#pragma once
#include <cstring>
#include "pico/stdlib.h"

#include "midi_processor_settings_view.h"
#include "midi_processor_mc_fader_pickup.h"
namespace rppicomidi
{
class Midi_processor_mc_fader_pickup_settings_view : public Midi_processor_settings_view
{
public:
    Midi_processor_mc_fader_pickup_settings_view()=delete;
    virtual ~Midi_processor_mc_fader_pickup_settings_view()=default;
    Midi_processor_mc_fader_pickup_settings_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_);
    void draw() final;
    static Midi_processor_settings_view* static_make_new(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_)
    {
        return new Midi_processor_mc_fader_pickup_settings_view(screen_, rect_, proc_);
    }
private:
    Midi_processor_mc_fader_pickup* mc_fader_pickup;
};
}