#pragma once
#include <cstring>
#include "pico/stdlib.h"

#include "menu.h"
#include "int_spinner_menu_item.h"
#include "midi_processor_settings_view.h"
#include "midi_processor_transpose.h"
namespace rppicomidi
{
class Midi_processor_transpose_view : public Midi_processor_settings_view
{
public:
    Midi_processor_transpose_view()=delete;
    virtual ~Midi_processor_transpose_view()=default;
    Midi_processor_transpose_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_);
    void draw() final;

    void entry() final {menu.entry();}
    void exit() final {menu.exit();}
    Select_result on_select(View** new_view) final { return menu.on_select(new_view);}
    void on_increment(uint32_t delta) final { menu.on_increment(delta); }
    void on_decrement(uint32_t delta) final { menu.on_decrement(delta); }

    static Midi_processor_settings_view* static_make_new(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_)
    {
        return new Midi_processor_transpose_view(screen_, rect_, proc_);
    }
private:
    Menu menu;
    Int_spinner_menu_item<uint8_t> chan;
    Int_spinner_menu_item<uint8_t> min_note;
    Int_spinner_menu_item<uint8_t> max_note;
    Int_spinner_menu_item<int8_t> transpose_delta;
};
}