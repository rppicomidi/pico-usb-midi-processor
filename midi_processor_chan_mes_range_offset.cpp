/* MIT License
 *
 * Copyright (c) 2026 rppicomidi
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

#include "midi_processor_chan_mes_range_offset.h"
#include "class/midi/midi.h"
rppicomidi::Midi_processor_chan_mes_range_offset::Midi_processor_chan_mes_range_offset(const char* name_, uint16_t unique_id) :
    Midi_processor{name_, unique_id}, note_off_msg{"Note Off"}, note_on_msg{"Note On"}, cc_msg{"CC"},
    poly_pressure_msg{"Poly Pressure"}, chan_pressure_msg{"Chan Pressure"}, prog_change_msg{"Prog Change"},
    format_decimal{"Display Decimal"}, format_hex{"Display Hex"},
    chan_in{"Channel In", 1, 16, 1},
    chan_out{"Channel In", 1, 16, 1},
    message_type_in{"Chan Mes In", {note_off_msg, note_on_msg, poly_pressure_msg, cc_msg, chan_pressure_msg, prog_change_msg}},
    message_type_out{"Chan Mes Out", {note_off_msg, note_on_msg, poly_pressure_msg, cc_msg, chan_pressure_msg, prog_change_msg}},
    byte1_range{"Byte1 Range",0,127},
    byte2_range{"Byte2 Range",0,127},
    byte1_offset{"Byte1 Offset", -127, 127, 0},
    byte2_offset{"Byte2 Offset", -127, 127, 0},
    display_format{"Display Format", {format_decimal, format_hex}}
{
    mutex_init(&processing_mutex);
    byte1_range.push_back(0,0);
    byte2_range.push_back(0,0);
}

bool rppicomidi::Midi_processor_chan_mes_range_offset::process(uint8_t* packet)
{
    uint8_t msg_chan = Midi_processor::get_channel_num(packet);
    if (msg_chan == chan_in.get()) {
        //printf("got channel match\r\n");
        // got a channel message on the right channel. See if it is the right type to process
        uint8_t status = (packet[1] >> 4) & 0xf;
        uint8_t in_msg_status = message_type_in.get_ivalue() + 0x8;
        uint8_t out_msg_status = message_type_out.get_ivalue() + 0x8;
        //printf("status=%02x in_msg_status=%02x\r\n",status, in_msg_status);
        if (status == in_msg_status)
        {
            //printf("got status match\r\n");
            // found the right channel message type. Are the bytes in range?
            if (packet[2] >= byte1_range.get(0,0) && packet[2] <= byte1_range.get(0,1) &&
                ((packet[3] >= byte2_range.get(0,0) && packet[3] <= byte2_range.get(0,1)) || 
                    status == MIDI_CIN_CHANNEL_PRESSURE  || status == MIDI_CIN_PROGRAM_CHANGE)) {
                //printf("got byte range match\r\n");
                mutex_enter_blocking(&processing_mutex);
                // Fix up the cin in byte 0 and remap the channel message status
                packet[0] &= 0xf0;
                packet[0] |= out_msg_status;
                packet[1] = (out_msg_status << 4) | (chan_out.get()-1);
                bool in_is_2_byte_msg = in_msg_status >= MIDI_CIN_PROGRAM_CHANGE;
                bool out_is_2_byte_msg = out_msg_status >= MIDI_CIN_PROGRAM_CHANGE;
                int byte1 = packet[2] + byte1_offset.get();
                if (byte1 < 0) {
                    byte1 = 0;
                }
                if (byte1 > 127) {
                    byte1 = 127;
                }
                int byte2 = packet[3] + byte2_offset.get();
                if (byte2 < 0) {
                    byte2 = 0;
                }
                if (byte2 > 127) {
                    byte2 = 127;
                }
                if (in_is_2_byte_msg == out_is_2_byte_msg) {
                    packet[2] = byte1;
                    if (!out_is_2_byte_msg) {
                        packet[3] = byte2;
                    }
                }
                else if (in_is_2_byte_msg) {
                    // Then the byte 2 offset value is the byte 1 value, and the computed offset byte1 value is the new byte 2
                    packet[2] = byte2_offset.get();
                    packet[3] = byte1;
                }
                else { // mapping a range of 3 byte message values to a single 2-byte message
                    // The byte 2 value is the new byte 1 value
                    packet[2] = byte2;
                    // packet[3] is unused
                }
                mutex_exit(&processing_mutex);
            }
        }
    }
    return true;
}

void rppicomidi::Midi_processor_chan_mes_range_offset::serialize_settings(const char* name, JSON_Object *root_object)
{
    JSON_Value *proc_value = json_value_init_object();
    JSON_Object *proc_object = json_value_get_object(proc_value);
    chan_in.serialize(proc_object);
    chan_out.serialize(proc_object);
    message_type_in.serialize(proc_object);
    message_type_out.serialize(proc_object);
    byte1_range.serialize(proc_object);
    byte2_range.serialize(proc_object);
    byte1_offset.serialize(proc_object);
    byte2_offset.serialize(proc_object);
    display_format.serialize(proc_object);
    json_object_set_value(root_object, name, proc_value);
    dirty = false;
}

bool rppicomidi::Midi_processor_chan_mes_range_offset::deserialize_settings(JSON_Object *root_object)
{
    bool result = false;
    if (chan_in.deserialize(root_object))
        result = true;

    if (!result || !chan_out.deserialize(root_object))
        result = false;

    if (!result || !message_type_in.deserialize(root_object))
        result = false;

    if (!result || !message_type_out.deserialize(root_object))
        result = false;

    if (!result || !byte1_range.deserialize(root_object))
        result = false;

    if (!result || !byte2_range.deserialize(root_object))
        result = false;

    if (!result || !byte1_offset.deserialize(root_object))
        result = false;

    if (!result || !byte2_offset.deserialize(root_object))
        result = false;

    if (!result || !display_format.deserialize(root_object))
        result = false;
    if (result)
        dirty = false;
    return result;
}