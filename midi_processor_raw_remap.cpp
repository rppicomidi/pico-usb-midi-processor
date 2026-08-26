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
#include "midi_processor_raw_remap.h"
#include "parson.h"

bool rppicomidi::Midi_processor_raw_remap::process(uint8_t* packet)
{
    // byte 0 of the packet is code index number/cable number byte
    if (packet[1] == in_mes.get(0) && packet[2] == in_mes.get(1) && packet[3] == in_mes.get(2)) {
        packet[1] = out_mes.get(0);
        packet[2] = out_mes.get(1);
        packet[3] = out_mes.get(2);
    }
    return true;
}

void rppicomidi::Midi_processor_raw_remap::serialize_settings(const char* name, JSON_Object *root_object)
{
    JSON_Value *proc_value = json_value_init_object();
    JSON_Object *proc_object = json_value_get_object(proc_value);
    display_format.serialize(proc_object);
    in_mes.serialize(proc_object);
    out_mes.serialize(proc_object);
    json_object_set_value(root_object, name, proc_value);
    dirty = false;
}

bool rppicomidi::Midi_processor_raw_remap::deserialize_settings(JSON_Object *root_object)
{
    bool result = false;
    if (display_format.deserialize(root_object))
        result = true;

    if (!result || !in_mes.deserialize(root_object))
        result = false;

    if (!result || !out_mes.deserialize(root_object))
        result = false;

    if (result) {
        dirty = false;
    }
    return result;
}

void rppicomidi::Midi_processor_raw_remap::load_defaults()
{
    display_format.set_default();
    in_mes.set_default();
    out_mes.set_default();
    dirty = false;
}
