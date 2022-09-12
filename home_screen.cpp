/**
 * @file mc_seven_seg_display.cpp
 * @brief This class implements the Assignment 2-digit seven segment display
 * and the Timecode/BBT 10-digit seven segment display. Per digit decimal
 * point display is not supported
 * 
 * Copyright (c) 2022 rppicomidi
 * 
 * The MIT License (MIT)
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
#include <cstring>
#include <cstdio>
#include "home_screen.h"

rppicomidi::Home_screen::Home_screen(View_manager& view_manager_, Mono_graphics& screen_, 
                            const char* device_label_, Setup_menu& setup_menu_) :
    View{screen_, screen_.get_clip_rect()},
    view_manager{view_manager_}, label_font{screen.get_font_12()},
    setup_menu{setup_menu_}
{
    set_device_label(device_label_);
}

void rppicomidi::Home_screen::center_text(const char* text_, uint8_t y_)
{
    auto text_len = strlen(text_);
    int x = screen.get_screen_width()/2 - (text_len*label_font.width)/2;
    screen.draw_string(label_font, x, y_, text_, text_len, Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);

}

void rppicomidi::Home_screen::draw()
{
    if (view_manager.is_current_view(this)) {
        screen.clear_canvas();
        auto device_label_len = strlen(device_label);
        if (device_label_len <= max_line_length) {
            // Center the Produce String on the 2nd line of the screen
            center_text(device_label, label_font.height);
        }
        else {
            // Break the device_label string into two lines.
            char line1[max_line_length+1];
            char line2[max_line_length+1];
            // Copy as much of the text onto the first line as possible and copy the remaining
            // text to the next line
            strncpy(line1, device_label, max_line_length);
            line1[max_line_length] = '\0';
            strncpy(line2, device_label+max_line_length, max_line_length);
            line2[max_line_length] = '\0';

            // See if we can break the text at a space
            char* ptr = strrchr(line1, ' ');
            bool center = false;
            if (ptr != nullptr) {
                // Found the last space
                char* remaining_text = device_label + (ptr - line1 + 1);
                if (strlen(remaining_text) <= max_line_length) {
                    // Terminate line 1 at the last space
                    *ptr = '\0';
                    // copy the remaining text to line 2
                    strncpy(line2, remaining_text, max_line_length);
                    line2[max_line_length] = '\0';
                    center = true; // center both lines of text for a cleaner look
                }                
            }
            if (center) {
                center_text(line1, 0);
                center_text(line2, label_font.height);
            }
            else {
                screen.draw_string(label_font, 0, 0, line1, strlen(line1), Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
                screen.draw_string(label_font, 0, label_font.height, line2, strlen(line2), Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
            }
        }
    }
}

rppicomidi::Home_screen::Select_result rppicomidi::Home_screen::on_select()
{
    printf("setup menu requested\r\n");
    view_manager.push_view(&setup_menu);
    return new_view; // the top of the view stack should remain unchanged
}

void rppicomidi::Home_screen::set_device_label(const char* device_label_)
{
    strncpy(device_label, device_label_, max_device_label);
    device_label[max_device_label] = '\0';
}