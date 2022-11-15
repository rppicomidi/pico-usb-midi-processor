/**
 * @file preset_view.cpp
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
#include "preset_view.h"

uint8_t rppicomidi::Preset_view::static_get_next_preset(void* context_)
{
    auto me = reinterpret_cast<Preset_view*>(context_);
    return me->next_preset.get();
}

uint8_t rppicomidi::Preset_view::static_incr_next_preset(void* context_, int delta)
{
    auto me = reinterpret_cast<Preset_view*>(context_);
    return me->next_preset.incr(delta);
}

void rppicomidi::Preset_view::draw()
{
    screen.clear_canvas();
    char str[21];
    sprintf(str, "Current Preset:%1x%s", Midi_processor_manager::instance().get_current_preset(), Midi_processor_manager::instance().needs_store() ? "[M]":"");
    screen.center_string(screen.get_font_12(), str, 0);
    menu.draw();
}

void rppicomidi::Preset_view::static_load_callback(View* context_, View**)
{
    auto me = reinterpret_cast<Preset_view*>(context_);
    Midi_processor_manager::instance().load_preset(me->next_preset.get());
    me->draw();
}

void rppicomidi::Preset_view::static_save_callback(View* context_, View**)
{
    auto me = reinterpret_cast<Preset_view*>(context_);
    Midi_processor_manager::instance().store_preset(me->next_preset.get());
    me->draw();
}

void rppicomidi::Preset_view::static_reset_callback(View* context_, View**)
{
    auto me = reinterpret_cast<Preset_view*>(context_);
    Midi_processor_manager::instance().clear_all_processors();
    Midi_processor_manager::instance().store_preset(me->next_preset.get());
    me->draw();
}