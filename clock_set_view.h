/**
 * @file clock_set_view.h
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
#pragma once

#include "view.h"
#include "rp2040_rtc.h"
#include "mono_graphics_lib.h"
#include "view_manager.h"
namespace rppicomidi
{
class Clock_set_view : public View
{
public:
    Clock_set_view(Mono_graphics& screen_, const Rectangle& rect_) : 
        View{screen_, rect_}, item_idx{0}, font{screen.get_font_12()} {}
    void draw();
    void entry() final;
    Select_result on_select(View** new_view) final;
    void on_increment(uint32_t delta, bool is_shifted) final;
    void on_decrement(uint32_t delta, bool is_shifted) final;
    void on_left(uint32_t delta, bool is_shifted) final;
    void on_right(uint32_t delta, bool is_shifted) final;
private:
    void draw_field(uint8_t field_idx, bool selected);
    uint8_t month, day, hour, minute, second;
    uint16_t year;
    uint8_t item_idx;
    static const uint8_t month_idx=0;
    static const uint8_t day_idx=1;
    static const uint8_t year_idx=2;
    static const uint8_t hour_idx=3;
    static const uint8_t min_idx=4;
    static const uint8_t sec_idx=5;
    static const uint8_t ok_idx=6;
    static const uint8_t x_offset = 7; // to center text for the date and time

    const Mono_mono_font& font;
};
}