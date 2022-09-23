#include <cstdio>
#include "midi_processing_setup_screen.h"
#include "midi_processor_manager.h"

rppicomidi::Midi_processing_setup_screen::Midi_processing_setup_screen(Mono_graphics& screen_, const Mono_mono_font& font_, 
        uint8_t cable_num_, bool is_midi_in_) :
        View{screen_, screen_.get_clip_rect()}, font{font_}, cable_num{cable_num_}, is_midi_in{is_midi_in_},
        menu{screen_, font.height, font},
        processor_select{screen_, font.height, this, select_callback}
{
    size_t num_processors = Midi_processor_manager::instance().get_num_midi_processor_types();
    for (size_t idx = 0; idx < num_processors; idx++) {
        auto item = new Menu_item{Midi_processor_manager::instance().get_midi_processor_name_by_idx(idx), screen, font};
        assert(item);
        processor_select.add_menu_item(item);
    }
    auto item = new View_launch_menu_item(processor_select,"Add new processor...", screen, font);
    assert(item);
    menu.add_menu_item(item);
}

void rppicomidi::Midi_processing_setup_screen::entry()
{
    menu.entry();
}

void rppicomidi::Midi_processing_setup_screen::draw()
{
    screen.clear_canvas();
    char str[21];
    sprintf(str, "MIDI %s %u Setup", is_midi_in ? "IN":"OUT", cable_num+1);
    screen.center_string(font, str, 0);
    menu.draw();
}

void rppicomidi::Midi_processing_setup_screen::select_callback(rppicomidi::View* view, int& idx)
{
    auto me = reinterpret_cast<Midi_processing_setup_screen*>(view);
    // TODO need to push a View_launch_menu_item with settings for each MIDI processor object, not just text selection
    me->menu.insert_menu_item_before_current(new Menu_item(me->processor_select.get_menu_item_text(idx), me->screen, me->font));
    Midi_processor_manager::instance().add_new_midi_processor_by_idx(idx, me->cable_num, me->is_midi_in);
}

rppicomidi::View::Select_result rppicomidi::Midi_processing_setup_screen::on_select(View** new_view)
{
    int idx = menu.get_current_item_idx();
    if (idx == static_cast<int>(menu.get_num_items())-1)
        return menu.on_select(new_view);
    printf("Selecting processor %s for edit or move\r\n", menu.get_menu_item_text(idx));
    return Select_result::no_op;
}