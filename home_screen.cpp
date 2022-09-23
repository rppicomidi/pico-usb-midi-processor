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

rppicomidi::Home_screen::Home_screen(View_manager& view_manager_, Mono_graphics& screen_, mutex& processor_mutex_,
    std::vector<std::vector<Midi_processor*>>& midi_in_processors_,
    std::vector<std::vector<Midi_processor*>>& midi_out_processors_,
    Midi_processor_factory& factory_, const char* device_label_) :
    View{screen_, screen_.get_clip_rect()},
    view_manager{view_manager_}, processor_mutex{processor_mutex_},
    midi_in_processors{midi_in_processors_}, midi_out_processors{midi_out_processors_},
    factory{factory_}, label_font{screen.get_font_12()},
    menu{screen, static_cast<uint8_t>(label_font.height*2+4), label_font}
{
    set_connected_device(device_label_, midi_in_processors.size(), midi_out_processors.size());
}


void rppicomidi::Home_screen::draw()
{
    if (view_manager.is_current_view(this)) {
        screen.clear_canvas();
        auto device_label_len = strlen(device_label);
        if (device_label_len <= max_line_length) {
            // Center the Produce String on the 2nd line of the screen
            screen.center_string(label_font, device_label, label_font.height);
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
                screen.center_string(label_font, line1, 0);
                screen.center_string(label_font, line2, label_font.height);
            }
            else {
                screen.draw_string(label_font, 0, 0, line1, strlen(line1), Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
                screen.draw_string(label_font, 0, label_font.height, line2, strlen(line2), Pixel_state::PIXEL_ONE, Pixel_state::PIXEL_ZERO);
            }
        }
        menu.draw();
    }
}

rppicomidi::Home_screen::Select_result rppicomidi::Home_screen::on_select(View** new_view)
{
    return menu.on_select(new_view);
}

void rppicomidi::Home_screen::set_connected_device(const char* device_label_, uint8_t num_in_cables_, uint8_t num_out_cables_)
{
    // Draw the product string
    strncpy(device_label, device_label_, max_device_label);
    device_label[max_device_label] = '\0';
    num_in_cables = num_in_cables_;
    num_out_cables = num_out_cables_;
    for (uint8_t cable=0; cable < num_in_cables; cable++)
        midi_in_setup.push_back(new Midi_processing_setup_screen{screen, label_font, processor_mutex, factory, midi_in_processors[cable], cable, true});
    for (uint8_t cable=0; cable < num_out_cables; cable++)
        midi_out_setup.push_back(new Midi_processing_setup_screen{screen, label_font, processor_mutex, factory, midi_out_processors[cable], cable, false});
    printf("New connection %s %u IN %u OUT\r\n", device_label, num_in_cables, num_out_cables);
    if (num_in_cables !=0 || num_out_cables !=0) {
        for (int port=0; port<num_in_cables; port++) {
            char line[max_line_length+1];
            sprintf(line,"Setup MIDI IN %u", port+1);
            Menu_item* item = new View_launch_menu_item(*midi_in_setup.at(port),line, screen, label_font);
            menu.add_menu_item(item);
        }
        for (int port=0; port<num_out_cables; port++) {
            char line[max_line_length+1];
            sprintf(line,"Setup MIDI OUT %u", port+1);
            Menu_item* item = new View_launch_menu_item(*midi_out_setup.at(port),line, screen, label_font);
            menu.add_menu_item(item);
        }
        // Treat the menu as a sub-view of this view. Do not change view.
        menu.entry();
        draw();
    }
}