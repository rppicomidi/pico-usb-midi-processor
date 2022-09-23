/**
 * @file midi_processor.h
 * @brief this is an abstract class the represents an arbitrary MIDI processor
 * and its settings. Settings are stored as JSON objects to make storing and
 * loading the settings to flash simpler.
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
#include <cstdint>
#include <cstring>
#include <cstdio>
namespace rppicomidi
{
class Midi_processor
{
public:
    Midi_processor(const char* name_, uint16_t unique_id_) : unique_id{unique_id_}
    {
        strncpy(name,name_,max_name_length);
        name[max_name_length] = '\0';
        if (has_feedback_process()) {
            snprintf(feedback_name,max_name_length,"fb-%s", name_);
            feedback_name[max_name_length] = '\0';
        }
        else {
            feedback_name[0] = '\0';
        }
    }

    /**
     * @brief Get the unique id object
     * 
     * @return uint16_t the unique ID
     */
    uint16_t get_unique_id() {return unique_id; }

    /**
     * @brief Get the name of the process
     *
     * @return const char* pointer to the process name
     */
    const char* get_name() {return name; }

    const char* get_feedback_name() { return has_feedback_process() ? feedback_name : nullptr; }
    /**
     * @brief perform the main processing from this input
     *
     * @param packet a pointer to the USB MIDI data packet to process 
     * @return true if data should be transmitted on
     * @return false to filter out the data
     */
    virtual bool process(uint8_t* packet) {(void)packet; return true; }

    /**
     * @brief Determine if this processor has a feedback process
     * 
     * For example, many controllers send a Note On or CC message to a DAW to indicate a button press.
     * The DAW will send back a Note On or CC message to control the light under the button. If you
     * remap the Note number of the CC number that the controller sends, you have to remap the number
     * the DAW sends to control the button light in the other direction. The process() method does
     * the remapping of the message from the controller, and the feedback() method does the opposite
     * remapping of the message from the DAW.
     *
     * For another example, a controller with moving faders both sends fader positions to the DAW
     * and receives fader positions from the DAW. A processor that simulates moving faders needs
     * to both send a receive data.
     *
     * @return true if this processor has a feedback process
     * @note usually there is no feedback process;
     */
    virtual bool has_feedback_process() {return false; }

    /**
     * @brief perform processing of a feedback message from the opposite direction
     *
     * @return true if data should be transmitted on
     * @return false
     * @note usually there is no feedback process;
     */
    virtual bool feedback(uint8_t packet[4]) {(void)packet; return false;}

    /**
     * @brief determine if this process has a periodic process task
     *
     * @return true if the task() method does anything
     */
    bool has_task() {return false; }

    /**
     * @brief Perform any periodic processing that this process needs
     * 
     */
    virtual void task() {}

    /**
     * @brief Determine if any of the processor's settings have changed since
     * they were last loaded or serialized
     *
     * This is useful to determine if it is necessary to save the settings to
     * flash
     *
     * @return true if settings need saving
     * @return false if settings have not changed
     */
    bool not_saved() { return dirty; }

    /**
     * @brief Serialize the settings for this process in JSON format
     *
     * @param settings_string the settings for this process in JSON format
     * @param max_settings_string is the maximum number of characters
     * that can be written to settings_string
     * @return true if the serialization is successful
     * @note calling this function clears the dirty flag
     */
    bool serialize_settings(char* settings_string);

    /**
     * @brief load the JSON formatted settings to the processor
     * 
     * @param settings_string the JSON formatted settings string for this processor
     * @return true if deserialization is successful
     */
    bool deserialize_settings(const char* settings_string);

    /**
     * @brief
     * 
     * @param packet the 4-byte USB MIDI packet
     * @return uint8_t the virtual MIDI cable 0-15
     */
    static uint8_t get_cable_num(uint8_t packet[4])
    {
        return (packet[0] >> 4) & 0xf;
    }

    /**
     * @brief Get the channel num object
     * 
     * @param packet the 4-byte USB MIDI packet
     * @return uint8_t 0 if not a channel message or the
     * MIDI channel number 1-16
     */
    static uint8_t get_channel_num(uint8_t packet[4])
    {
        uint8_t channel = 0;
        if (packet[1]>= 0x80 && packet[0] <= 0xEF)
        {
            channel = (packet[1] & 0xf) + 1;
        }
        return channel;
    }
protected:
    static const uint8_t max_name_length=21;
    char name[max_name_length+1];
    char feedback_name[max_name_length+1];
    bool dirty; // if true, then the settings need to be saved 
    uint16_t unique_id;
};

/**
 * @brief Midi_processor ptr with a flag to choose whether to call the process() 
 * or feedback() function
 */
struct Midi_processor_fn
{   
    Midi_processor* proc;   //!< pointer to the Midi_processor whose process() or feedback() function is called
    bool is_feedback;       //!< if true, call proc->feedback(); otherwise call proc->process().
};
}