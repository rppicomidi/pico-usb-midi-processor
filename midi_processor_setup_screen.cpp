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
#include <cstdio>
#include <cstdlib>
#include "midi_processor_setup_screen.h"
#include "midi_processor_manager.h"

rppicomidi::Midi_processor_setup_screen::Midi_processor_setup_screen(Mono_graphics& screen_, const Mono_mono_font& font_, 
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
}

void rppicomidi::Midi_processor_setup_screen::entry()
{
    menu.clear();
    auto item = new View_launch_menu_item(processor_select,"Add new processor...", screen, font);
    assert(item);
    menu.add_menu_item(item);

    // Build the current processor list
    size_t nprocessors = Midi_processor_manager::instance().get_num_midi_processors(cable_num, is_midi_in);
    for (size_t idx = 0; idx < nprocessors; idx++) {
        auto proc = Midi_processor_manager::instance().get_midi_processor_by_index(idx, cable_num, is_midi_in);
        auto newview = Midi_processor_manager::instance().get_midi_processor_view_by_index(idx, cable_num, is_midi_in);
        if (proc && newview) {
            auto proc_item = new View_launch_menu_item(*newview, proc->get_name(), screen, font);
            menu.insert_menu_item_before_current(proc_item);
            menu.set_current_item_idx(idx+1);
        }
    }
    menu.entry();
}

void rppicomidi::Midi_processor_setup_screen::exit()
{
    menu.exit();
    menu.clear();
}

void rppicomidi::Midi_processor_setup_screen::draw()
{
    screen.clear_canvas();
    char str[21];
    sprintf(str, "MIDI %s %u Setup", is_midi_in ? "IN":"OUT", cable_num+1);
    screen.center_string(font, str, 0);
    menu.draw();
}

void rppicomidi::Midi_processor_setup_screen::select_callback(rppicomidi::View* view, int idx)
{
    auto me = reinterpret_cast<Midi_processor_setup_screen*>(view);
    auto newview = Midi_processor_manager::instance().add_new_midi_processor_by_idx(idx, me->cable_num, me->is_midi_in);
    me->menu.insert_menu_item_before_current(new View_launch_menu_item(*newview, me->processor_select.get_menu_item_text(idx), me->screen, me->font));
}

rppicomidi::View::Select_result rppicomidi::Midi_processor_setup_screen::on_select(View** new_view)
{
    int idx = menu.get_current_item_idx();
    if (idx <= static_cast<int>(menu.get_num_items())-1) {
        return menu.on_select(new_view);
    }
    return Select_result::no_op;
}

void rppicomidi::Midi_processor_setup_screen::on_left(uint32_t delta, bool is_shifted)
{
    // require shift button pressed to delete a processor && delta == 1
    // to prevent accidental deletions
    if (delta != 1 || !is_shifted) {
        return;
    }

    int idx = menu.get_current_item_idx();
    if (idx == -1 || idx == static_cast<int>(menu.get_num_items())-1) {
        return; // either the menu is empty (which is bad) or pointing to the add new item menu item
    }
    Midi_processor_manager::instance().delete_midi_processor_by_idx(idx, cable_num, is_midi_in);

    menu.erase_current_item();
    menu.draw();
}