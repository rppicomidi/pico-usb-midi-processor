#include <cstring>
#include "midi_processor_transpose_view.h"
rppicomidi::Midi_processor_transpose_view::Midi_processor_transpose_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_) :
        Midi_processor_settings_view{screen_, rect_, proc_},
        menu{screen, screen.get_font_12().height, screen.get_font_12()},
        chan{"MIDI chan: ",screen,screen.get_font_12(),2,false,Midi_processor_transpose::static_get_chan, Midi_processor_transpose::static_incr_chan, proc_},
        min_note{"Min MIDI note: ", screen, screen.get_font_12(), 3, false, Midi_processor_transpose::static_get_min_note, Midi_processor_transpose::static_incr_min_note, proc_},
        max_note{"Max MIDI note: ", screen, screen.get_font_12(), 3, false, Midi_processor_transpose::static_get_max_note, Midi_processor_transpose::static_incr_max_note, proc_},
        transpose_delta{"Halfstep delta: ", screen, screen.get_font_12(), 3, false, Midi_processor_transpose::static_get_transpose_delta, Midi_processor_transpose::static_incr_transpose_delta, proc_}
{
    // Make sure that the proc points to a Midi_processor_transpose object (this c++ does not have dynamic cast)
    size_t name_len = strlen(proc->get_name());
    assert(name_len == strlen(Midi_processor_transpose::static_getname()));
    assert(strcmp(proc->get_name(),Midi_processor_transpose::static_getname()) == 0);
    menu.add_menu_item(&chan);
    menu.add_menu_item(&min_note);
    menu.add_menu_item(&max_note);
    menu.add_menu_item(&transpose_delta);
}

void rppicomidi::Midi_processor_transpose_view::draw()
{
    screen.clear_canvas();
    screen.center_string(screen.get_font_12(), "Transpose Settings", 0);
    menu.draw();
}