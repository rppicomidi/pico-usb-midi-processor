#pragma once
#include "view.h"
#include "midi_processor.h"
namespace rppicomidi
{
class Midi_processor_settings_view : public View
{
public:
    Midi_processor_settings_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_) :
        View{screen_, rect_}, proc{proc_} {}

    virtual ~Midi_processor_settings_view()=default;
protected:
    Midi_processor* proc;
};
}