#pragma once
#include <vector>
#include "pico/mutex.h"
#include "view.h"
#include "menu.h"
#include "view_launch_menu_item.h"
#include "text_item_chooser_menu.h"
#include "midi_processor_factory.h"
#include "midi_processor.h"
namespace rppicomidi
{
class Midi_processing_setup_screen : public View
{
public:
    Midi_processing_setup_screen(Mono_graphics& screen_, const Mono_mono_font& font_,
        uint8_t cable_num_, bool is_midi_in_);
    virtual void draw();
    static void select_callback(View* me, int& idx);
    virtual void entry();
    Select_result on_select(View** new_view) final;
    void on_increment(uint32_t delta) final {menu.on_increment(delta); };
    void on_decrement(uint32_t delta) final {menu.on_decrement(delta); };
private:
    Mono_mono_font font;
    uint8_t cable_num;
    bool is_midi_in;
    Menu menu;
    Text_item_chooser_menu processor_select;
    std::vector<const char*> processor_names;
};
}