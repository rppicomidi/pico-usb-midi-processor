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

#include "midi_processor_pitchbend_to_cc.h"
#include "class/midi/midi.h"
rppicomidi::Midi_processor_pitchbend_to_cc::Midi_processor_pitchbend_to_cc(uint16_t unique_id, Midi_buffer* proc_buf_, Thread_safe_midi_buffer* in_buf_, Midi_buffer* fb_buf_, Thread_safe_midi_buffer* out_buf_) :
    Midi_processor{static_getname(), unique_id, proc_buf_, in_buf_, fb_buf_, out_buf_},
    format_decimal{"Display Decimal"}, format_hex{"Display Hex"},
    resolution7{"Resolution: 7-bit"}, resolution14{"Resolution: 14-bit"},
    resolution{"CC Resloution", {resolution7, resolution14}},
    bimap{"PB chan->CC",0,128 /* if the remap is 128, it means the message packet should be filtered out */ },
    display_format{"Display Format", {format_decimal, format_hex}},
    chan{"CC Channel", 1, 16, 1}

{
    bimap.set_min(1, 0);
    bimap.set_max(17, 0);
    mutex_init(&processing_mutex);
}

bool rppicomidi::Midi_processor_pitchbend_to_cc::process_one(Midi_packet& pkt)
{
    if (pkt.getChanMesCin() == MIDI_CIN_PITCH_BEND_CHANGE) {
        // pitch bend message; check if channel is in range
        int idx = bimap.find(pkt.getChanMesChannel(), 0);
        if (idx != -1) {
            // found the channel; get the corresponding CC number for the MSB
            uint8_t cc = bimap.get(idx, 1);
            if (cc < bimap.get_max(1)) {
                Midi_packet cc_pkt;
                cc_pkt.setChanMesCin(MIDI_CIN_CONTROL_CHANGE);
                cc_pkt.setChanMesChannel(chan.get());
                cc_pkt.setData1(cc);
                cc_pkt.setData2(pkt.getData2());
                proc_buf->push(cc_pkt);
                if (get_resolution() == 1) {
                    cc_pkt.setData1(cc+32);
                    cc_pkt.setData2(pkt.getData1());
                    proc_buf->push(cc_pkt);
                }
                return false; // proc_buf is managed locally
            }
        }
    }
    return true; // push pkt unchanged
}

void rppicomidi::Midi_processor_pitchbend_to_cc::serialize_settings(const char* name, JSON_Object *root_object)
{
    JSON_Value *proc_value = json_value_init_object();
    JSON_Object *proc_object = json_value_get_object(proc_value);
    chan.serialize(proc_object);
    bimap.serialize(proc_object);
    display_format.serialize(proc_object);
    resolution.serialize(proc_object);
    json_object_set_value(root_object, name, proc_value);
    dirty = false;
}

bool rppicomidi::Midi_processor_pitchbend_to_cc::deserialize_settings(JSON_Object *root_object)
{
    bool result = false;
    if (chan.deserialize(root_object))
        result = true;

    if (!result || !bimap.deserialize(root_object))
        result = false;

    if (!result || !display_format.deserialize(root_object))
        result = false;

    if (!result || !resolution.deserialize(root_object))
        result = false;

    if (result)
        dirty = false;
    return result;
}