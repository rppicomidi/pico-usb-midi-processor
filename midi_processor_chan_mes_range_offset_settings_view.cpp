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
#include "midi_processor_chan_mes_range_offset_settings_view.h"
#include "int_spinner_menu_item.h"
rppicomidi::Midi_processor_chan_mes_range_offset_settings_view::
    Midi_processor_chan_mes_range_offset_settings_view(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_) :
        Midi_processor_settings_view{screen_, rect_, proc_}, font{screen_.get_font_12()}, menu{screen_, font.height, font},
        mes_in_type_menu{screen_, 0, this, static_mes_type_in_select_callback},
        mes_out_type_menu{screen_, 0, this, static_mes_type_out_select_callback},
        mes_in_type_menu_item{nullptr}, mes_out_type_menu_item{nullptr}, display_format_item{nullptr}
{
    auto remap_proc = reinterpret_cast<Midi_processor_chan_mes_range_offset*>(proc);
    auto mes_type_list = remap_proc->get_all_possible_channel_message_types();
    assert(mes_type_list);
    for (auto &mes_type: *mes_type_list) {
        auto item_in = new Menu_item{mes_type.c_str(), screen, font};
        assert(item_in);
        mes_in_type_menu.add_menu_item(item_in);
        auto item_out = new Menu_item{mes_type.c_str(), screen, font};
        assert(item_out);
        mes_out_type_menu.add_menu_item(item_out);
    }
}

void rppicomidi::Midi_processor_chan_mes_range_offset_settings_view::entry()
{
    auto remap_proc = reinterpret_cast<Midi_processor_chan_mes_range_offset*>(proc);
    auto mes_type_list = remap_proc->get_all_possible_channel_message_types();
    menu.clear();
    std::string display_format_str;
    remap_proc->get_display_format(display_format_str);
    display_format_item = new Callback_menu_item{display_format_str.c_str(), screen, font, this, static_toggle_display_format};
    assert(display_format_item);
    menu.add_menu_item(display_format_item);
    auto chan_in_menu_item = new Int_spinner_menu_item<uint8_t>("In Channel:",screen, font, 3, 3, false,
            Midi_processor_chan_mes_range_offset::static_chan_in_get,
            Midi_processor_chan_mes_range_offset::static_chan_in_incr, reinterpret_cast<void*>(proc));
    assert(chan_in_menu_item);
    menu.add_menu_item(chan_in_menu_item);
    auto chan_out_menu_item = new Int_spinner_menu_item<uint8_t>("Out Channel:",screen, font, 3, 3, false,
            Midi_processor_chan_mes_range_offset::static_chan_out_get,
            Midi_processor_chan_mes_range_offset::static_chan_out_incr, reinterpret_cast<void*>(proc));
    assert(chan_out_menu_item);
    menu.add_menu_item(chan_out_menu_item);
    const char* text_in_msg_type =( std::string(" In:") + ((*mes_type_list)[mes_in_type_menu.get_current_item_idx()])).c_str();
    mes_in_type_menu_item = new View_launch_menu_item(mes_in_type_menu, text_in_msg_type, screen, font);
    assert(mes_in_type_menu_item);
    menu.add_menu_item(mes_in_type_menu_item);
    const char* text_out_msg_type =( std::string("Out:") + ((*mes_type_list)[mes_out_type_menu.get_current_item_idx()])).c_str();
    mes_out_type_menu_item = new View_launch_menu_item(mes_out_type_menu, text_out_msg_type, screen, font);
    assert(mes_out_type_menu_item);
    menu.add_menu_item(mes_out_type_menu_item);
    size_t fmt_idx = remap_proc->get_display_format();

    auto range1_item = new Bimap_spinner_menu_item<uint8_t>("Byte1 In:", screen, font, 0, 3, 2, fmt_idx == 1,
        Midi_processor_chan_mes_range_offset::static_get_byte1_range, Midi_processor_chan_mes_range_offset::static_incr_byte1_range,
        reinterpret_cast<void*>(remap_proc));
    menu.add_menu_item(range1_item);
    auto offset1_item = new Int_spinner_menu_item<int8_t>("Byte1 Out Offset:",screen, font, 3, 3, false,
            Midi_processor_chan_mes_range_offset::static_get_byte1_offset,
            Midi_processor_chan_mes_range_offset::static_incr_byte1_offset, reinterpret_cast<void*>(proc));
    menu.add_menu_item(offset1_item);
    auto range2_item = new Bimap_spinner_menu_item<uint8_t>("Byte2 In:", screen, font, 0, 3, 2, fmt_idx == 1,
        Midi_processor_chan_mes_range_offset::static_get_byte2_range, Midi_processor_chan_mes_range_offset::static_incr_byte2_range,
        reinterpret_cast<void*>(remap_proc));
    menu.add_menu_item(range2_item);
    auto offset2_item = new Int_spinner_menu_item<int8_t>("Byte2 Out Offset:",screen, font, 3, 3, false,
            Midi_processor_chan_mes_range_offset::static_get_byte2_offset,
            Midi_processor_chan_mes_range_offset::static_incr_byte2_offset, reinterpret_cast<void*>(proc));
    menu.add_menu_item(offset2_item);
    menu.entry();
}

void rppicomidi::Midi_processor_chan_mes_range_offset_settings_view::draw()
{
    screen.clear_canvas();
    screen.center_string(font, "Chan Mes Range Offset", 0);
    menu.draw();
}

void rppicomidi::Midi_processor_chan_mes_range_offset_settings_view::static_mes_type_in_select_callback(View* context, int idx)
{
    auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset_settings_view*>(context);
    auto remap_proc = reinterpret_cast<Midi_processor_chan_mes_range_offset*>(me->proc);
    if (!remap_proc->set_message_in_type(idx)) {
        remap_proc->set_message_in_type(0);
    }
    std::string remap_str;
    remap_proc->get_message_in_type(remap_str);
    me->mes_in_type_menu_item->set_text(remap_str.c_str());
}

void rppicomidi::Midi_processor_chan_mes_range_offset_settings_view::static_mes_type_out_select_callback(View* context, int idx)
{
    auto me = reinterpret_cast<Midi_processor_chan_mes_range_offset_settings_view*>(context);
    auto remap_proc = reinterpret_cast<Midi_processor_chan_mes_range_offset*>(me->proc);
    if (!remap_proc->set_message_out_type(idx)) {
        remap_proc->set_message_out_type(0);
    }
    std::string remap_str;
    remap_proc->get_message_out_type(remap_str);
    me->mes_out_type_menu_item->set_text(remap_str.c_str());
}

void rppicomidi::Midi_processor_chan_mes_range_offset_settings_view::static_toggle_display_format(View* context, View**)
{
    auto me=reinterpret_cast<Midi_processor_chan_mes_range_offset_settings_view*>(context);
    auto remap_proc = reinterpret_cast<Midi_processor_chan_mes_range_offset*>(me->proc);
    size_t fmt_idx = remap_proc->get_display_format();
    bool is_hex = (fmt_idx == 0); // toggle the display format value
    remap_proc->set_display_format(is_hex ? 1:0); // update the setting
    // update the display format menu item string
    std::string fmt_str;
    remap_proc->get_display_format(fmt_str);
    me->display_format_item->set_text(fmt_str.c_str());
#if 0
    // first 3 menu items are message type, channel and display format toggle
    // last menu item is add new remap
    for (int idx=4; idx < (int)me->menu.get_num_items()-1; idx++) {
        // then there are remap items to change format
        int ret = me->menu.set_current_item_idx(idx);
        assert(ret == idx);
        #ifdef NDEBUG
        (void)ret;
        #endif
        auto item = reinterpret_cast<Bimap_spinner_menu_item<uint8_t>*>(me->menu.get_current_item());
        item->set_display_hex(is_hex);
    }
#endif
    int ret = me->menu.set_current_item_idx(5);
    assert(ret == 5);
    auto item = reinterpret_cast<Bimap_spinner_menu_item<uint8_t>*>(me->menu.get_current_item());
    item->set_display_hex(is_hex);
    ret = me->menu.set_current_item_idx(7);
    assert(ret == 7);
    item = reinterpret_cast<Bimap_spinner_menu_item<uint8_t>*>(me->menu.get_current_item());
    item->set_display_hex(is_hex);
    me->menu.set_current_item_idx(0); // set the item index back to the display format
    me->draw();
    (void)ret;
}


void rppicomidi::Midi_processor_chan_mes_range_offset_settings_view::on_left(uint32_t delta, bool is_shifted)
{
    // pass it on to the current item via the menu object
    menu.on_left(delta, is_shifted);
}