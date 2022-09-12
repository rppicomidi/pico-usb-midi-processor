/**
 * @file mc_bridge_model.cpp
 * @brief this class is the MVC model for the configuration UI
 *
 * This class contains all the settings required by the Pico-usb-midi-processor
 * class, which is the controller in the MVC model.
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
#include <cstdio>
#include "midi_processor_model.h"
#include "parson.h"

rppicomidi::Midi_processor_model::Midi_processor_model() : needs_save{false}, num_cables_in{1}, num_cables_out{1}
{

}

void rppicomidi::Midi_processor_model::load_defaults()
{
    needs_save = true;
}

char* rppicomidi::Midi_processor_model::serialize() const
{
    char *serialized_string = NULL;
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    #if 0 // TODO
    json_object_set_number(root_object, "mc_in", mc_in);
    json_object_set_number(root_object, "mc_out", mc_out);
    json_object_set_value(root_object, "mc_note_mapping", json_value_init_array());
    JSON_Array *note_map = json_object_get_array(root_object, "mc_note_mapping");
    for (int note = 0; note < 128; note++) {
        json_array_append_number(note_map, mc_note_mapping[note]);
    }
    #endif
    json_set_float_serialization_format("%.0f");
    serialized_string = json_serialize_to_string(root_value);
    json_value_free(root_value);
    return serialized_string;
}

bool rppicomidi::Midi_processor_model::deserialize(const char* settings_str)
{
#if 0 // TODO
    JSON_Value *root_value = json_parse_string(settings_str);
    JSON_Object *root_object = NULL;
    if (root_value && json_value_get_type(root_value) == JSONObject) {
        root_object = json_value_get_object(root_value);
        double val;
        if (json_object_has_value_of_type(root_object, "mc_in", JSONNumber)) {
            val = json_object_get_number(root_object, "mc_in");
            if (val < (double)num_cables_in || val == 255.0) {
                mc_in = val;
            }
            else {
                printf("Val %g too big for mc_in; loading default\r\n", val);
                mc_in = 0;
            }
        }
        else {
            printf("Could not parse mc_in\r\n");
            json_value_free(root_value);
            return false;
        }
        if (json_object_has_value_of_type(root_object, "mc_out", JSONNumber)) {
            val = json_object_get_number(root_object, "mc_out");
            if (val < (double)num_cables_out || val == 255.0) {
                mc_out = val;
            }
            else {
                printf("Val %g too big for mc_out; loading default\r\n", val);
                mc_out = 0;
            }
        }
        else {
            printf("Could not parse mc_out\r\n");
            json_value_free(root_value);
            return false;
        }
        if (json_object_has_value_of_type(root_object, "mc_note_mapping", JSONArray)) {
            auto note_mapping = json_object_get_array(root_object, "mc_note_mapping");
            if (json_array_get_count(note_mapping) != 128) {
                json_value_free(root_value);
                printf("mc_note_mapping array too short\r\n");
                return false;
            }
            for (size_t note = 0; note < 128; note++) {
                val = json_array_get_number(note_mapping, note);
                if (val < 128.0) {
                    mc_note_mapping[note] = val;
                }
                else {
                    printf("note number %u mapping to illegal note number %g; setting to default\r\n", note, val);
                    mc_note_mapping[note] = note;
                }
            }
        }
        else {
            printf("Could not parse mc_note_mapping\r\n");
            json_value_free(root_value);
            return false;
        }

        json_value_free(root_value);
        return true;
    }

    if (root_value)
        json_value_free(root_value);
#endif
    return false;
}