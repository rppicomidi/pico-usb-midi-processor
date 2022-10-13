/**
 * @file midi_processor_chan_mes_remap_settings_view.cpp
 * @author rppicomidi
 * @brief Settings UI for channel message remap
 *
 * The UI looks like this:
 *      Note Message Remap *** This is a menu item; can select any Channel message type except pitch bend
 *      Channel: 1
 *      Display Decimal
 *      Remap: 54->---
 *      Remap:122->123
 *      Add New Remap
 * 
 * MIT License
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
#include <cassert>
#include <string>
#include "midi_processor_chan_mes_remap_settings_view.h"
#include "int_spinner_menu_item.h"
rppicomidi::Midi_processor_chan_mes_remap_settings_view::
    Midi_processor_chan_mes_remap_settings_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_) :
        Midi_processor_settings_view{screen_, rect_, proc_}, font{screen_.get_font_12()}, menu{screen_, 0, font},
        mes_type_menu{screen_, 0, this, static_mes_type_select_callback}
{
    auto remap_proc = reinterpret_cast<Midi_processor_chan_mes_remap*>(proc);
    auto mes_type_list = remap_proc->get_all_possible_channel_message_types();
    assert(mes_type_list);
    for (auto &mes_type: *mes_type_list) {
        auto item = new Menu_item{mes_type.c_str(), screen, font};
        assert(item);
        mes_type_menu.add_menu_item(item);
    }
    const char* text = (*mes_type_list)[mes_type_menu.get_current_item_idx()].c_str();
    mes_type_menu_item = new View_launch_menu_item(mes_type_menu, text, screen, font);
    assert(mes_type_menu_item);
    menu.add_menu_item(mes_type_menu_item);
    auto chan_menu_item = new Int_spinner_menu_item<uint8_t>("Channel:",screen, font, 3, 3, false,
            Midi_processor_chan_mes_remap::static_chan_get,
            Midi_processor_chan_mes_remap::static_chan_incr, reinterpret_cast<void*>(proc));
    assert(chan_menu_item);
    menu.add_menu_item(chan_menu_item);
    std::string display_format_str;
    remap_proc->get_display_format(display_format_str);
    display_format_item = new Callback_menu_item{display_format_str.c_str(), screen, font, this, static_toggle_display_format};
    assert(display_format_item);
    menu.add_menu_item(display_format_item);
    auto item = new Callback_menu_item{"Add new remap", screen, font, this, static_new_remap_callback};
    assert(item);
    menu.add_menu_item(item);
}

void rppicomidi::Midi_processor_chan_mes_remap_settings_view::draw()
{
    screen.clear_canvas();
    menu.draw();
}

void rppicomidi::Midi_processor_chan_mes_remap_settings_view::static_mes_type_select_callback(View* context, int& idx)
{
    auto me = reinterpret_cast<Midi_processor_chan_mes_remap_settings_view*>(context);
    auto remap_proc = reinterpret_cast<Midi_processor_chan_mes_remap*>(me->proc);
    if (!remap_proc->set_message_type(idx)) {
        remap_proc->set_message_type(0);
    }
    std::string remap_str;
    remap_proc->get_message_type(remap_str);
    me->mes_type_menu_item->set_text(remap_str.c_str());
}

void rppicomidi::Midi_processor_chan_mes_remap_settings_view::static_toggle_display_format(View* context)
{
    auto me=reinterpret_cast<Midi_processor_chan_mes_remap_settings_view*>(context);
    auto remap_proc = reinterpret_cast<Midi_processor_chan_mes_remap*>(me->proc);
    size_t fmt_idx = remap_proc->get_display_format();
    bool is_hex = (fmt_idx == 0); // toggle the display format value
    remap_proc->set_display_format(is_hex ? 1:0); // update the setting
    // update the display format menu item string
    std::string fmt_str;
    remap_proc->get_display_format(fmt_str);
    me->display_format_item->set_text(fmt_str.c_str());
    // first 3 menu items are message type, channel and display format toggle
    // last menu item is add new remap
    for (int idx=3; idx < (int)me->menu.get_num_items()-1; idx++) {
        // then there are remap items to change format
        int ret = me->menu.set_current_item_idx(idx);
        assert(ret == idx);
        auto item = reinterpret_cast<Bimap_spinner_menu_item<uint8_t>*>(me->menu.get_current_item());
        item->set_display_hex(is_hex);
    }
    me->menu.set_current_item_idx(2); // set the item index back to the display format
    me->draw();
}

void rppicomidi::Midi_processor_chan_mes_remap_settings_view::static_new_remap_callback(View* context)
{
    auto me=reinterpret_cast<Midi_processor_chan_mes_remap_settings_view*>(context);
    auto remap_proc = reinterpret_cast<Midi_processor_chan_mes_remap*>(me->proc);
    auto idx = remap_proc->add_remap();
    size_t fmt_idx = remap_proc->get_display_format();
    auto item = new Bimap_spinner_menu_item<uint8_t>("Remap:", me->screen, me->font, idx, 3, 2, fmt_idx == 1,
            Midi_processor_chan_mes_remap::static_get, Midi_processor_chan_mes_remap::static_incr,
            reinterpret_cast<void*>(remap_proc));
    me->menu.insert_menu_item_before_current(item);
    me->draw();
}

void rppicomidi::Midi_processor_chan_mes_remap_settings_view::on_left(uint32_t delta, bool is_shifted)
{
    if (menu.get_has_focus()) {
        // Do not erase multiple items at once by accident
        if (delta != 1)
            return;
        // Then delete the current menu item if it is a remap item
        size_t idx = menu.get_current_item_idx();
        // idx == 0 is the message type.
        // idx == 1 is the remap display format
        // idx == menu.get_num_items() - 1 is the add new remap prompt
        // If the current possition is between those, then it is a remap. Erase it
        if (idx > 1 && idx < menu.get_num_items()-1) {
            // first erase the associated processing mapping. The index to it is stored
            // in the current menu item. I thought about putting this functionality in
            // the Bimap_spinner_menu_item destructor, but it seemed more
            // consistent with the code that adds a new remap to do it here
            auto item = reinterpret_cast<const Bimap_spinner_menu_item<uint8_t>*>(menu.get_current_item());
            assert(item);
            auto bimap_idx = item->get_bimap_idx();
            auto remap_proc = reinterpret_cast<Midi_processor_chan_mes_remap*>(proc);
            remap_proc->delete_remap(bimap_idx);
            // Now erase the current menu item
            menu.erase_current_item();
            draw(); // need to redraw
        }
    }
    else {
        // pass it on to the current item via the menu object
        menu.on_left(delta, is_shifted);
    }
}