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

#include "midi_processor_chan_mes_remap.h"
#include "class/midi/midi.h"
rppicomidi::Midi_processor_chan_mes_remap::Midi_processor_chan_mes_remap(const char* name_, uint16_t unique_id) :
    Midi_processor{name_, unique_id}, note_msg{"Note Number Remap"}, cc_msg{"CC Number Remap"},
    poly_pressure_msg{"Poly Press Note Remap"}, chan_pressure_msg{"Chan Pressure Remap"}, prog_change_msg{"Program Number Remap"},
    format_decimal{"Display Decimal"}, format_hex{"Display Hex"},
    chan{"Channel", 1, 16, 1}, message_type{"Channel Message", {note_msg, cc_msg, poly_pressure_msg, chan_pressure_msg, prog_change_msg}},
    bimap{"remap",0,128 /* if the remap is 128, it means the message packet should be filtered out */ },
    display_format{"Display Format", {format_decimal, format_hex}}
{
    mutex_init(&processing_mutex);
}

#if 0
rppicomidi::Midi_processor_chan_mes_remap::Midi_processor_chan_mes_remap(uint16_t unique_id) :
    Midi_processor{static_getname(), unique_id}, note_msg{"Note Number Remap"}, cc_msg{"CC Number Remap"},
    poly_pressure_msg{"Poly Press Note Remap"}, chan_pressure_msg{"Chan Pressure Remap"}, prog_change_msg{"Program Number Remap"},
    format_decimal{"Display Decimal"}, format_hex{"Display Hex"},
    chan{"Channel", 1, 16, 1}, message_type{"Channel Message", {note_msg, cc_msg, poly_pressure_msg, chan_pressure_msg, prog_change_msg}},
    bimap{"remap",0,128 /* if the remap is 128, it means the message packet should be filtered out */ },
    display_format{"Display Format", {format_decimal, format_hex}}
{
    mutex_init(&processing_mutex);
}
#endif

bool rppicomidi::Midi_processor_chan_mes_remap::process_internal(uint8_t* packet, size_t first_idx, size_t second_idx)
{
    bool donotfilter = true;
    uint8_t msg_chan = Midi_processor::get_channel_num(packet);
    if (msg_chan == chan.get()) {      
        // got a channel message on the right channel. See if it is the right type to process
        uint8_t status = (packet[1] >> 4) & 0xf;
        if (((status == MIDI_CIN_NOTE_ON || status == MIDI_CIN_NOTE_ON) && message_type == note_msg) ||
            (status == MIDI_CIN_CONTROL_CHANGE && message_type == cc_msg) ||
            (status == MIDI_CIN_POLY_KEYPRESS && message_type == poly_pressure_msg) ||
            (status == MIDI_CIN_CHANNEL_PRESSURE && message_type == chan_pressure_msg) ||
            (status == MIDI_CIN_PROGRAM_CHANGE && message_type == prog_change_msg))
         {
            // found the right channel message type. Remap it
            mutex_enter_blocking(&processing_mutex);
            int idx = bimap.find(packet[2], first_idx);
            if (idx != -1) {
                uint8_t remap=bimap.get(idx, second_idx);
                if (remap == bimap.get_max()) {
                    // filter out this packet
                    donotfilter = false;
                }
                else {
                    packet[2]=remap;
                }
            }
            mutex_exit(&processing_mutex);
        }
    }
    return donotfilter;
}

void rppicomidi::Midi_processor_chan_mes_remap::serialize_settings(const char* name, JSON_Object *root_object)
{
    JSON_Value *proc_value = json_value_init_object();
    JSON_Object *proc_object = json_value_get_object(proc_value);
    chan.serialize(proc_object);
    message_type.serialize(proc_object);
    bimap.serialize(proc_object);
    display_format.serialize(proc_object);
    json_object_set_value(root_object, name, proc_value);
    dirty = false;
}

bool rppicomidi::Midi_processor_chan_mes_remap::deserialize_settings(JSON_Object *root_object)
{
    bool result = false;
    if (chan.deserialize(root_object))
        result = true;

    if (!result || !message_type.deserialize(root_object))
        result = false;

    if (!result || !bimap.deserialize(root_object))
        result = false;

    if (!result || !display_format.deserialize(root_object))
        result = false;
    if (result)
        dirty = false;
    return result;
}