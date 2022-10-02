#include "midi_processor_transpose.h"
#include "parson.h"

bool rppicomidi::Midi_processor_transpose::process(uint8_t* packet)
 {
    bool success = true; // Only block passing the message on if transposing makes the note out of MIDI range
    if (get_channel_num(packet) == chan.get()) {
        uint8_t status = packet[1] & 0xf0;
        if (status == 0x90 || status == 0x80) {
            // note message
            if (packet[2] >= min_note.get() && packet[2] <= max_note.get()) {
                int new_note = packet[2] + transpose_delta.get();
                if (new_note >= min_note.get() && new_note <= max_note.get()) {
                    packet[2] = static_cast<uint8_t>(new_note);
                }
                else {
                    success = false;
                }
            }
        }
    }
    return success;
}

char* rppicomidi::Midi_processor_transpose::serialize_settings()
{
    char *serialized_string = nullptr;
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    chan.serialize(root_object);
    min_note.serialize(root_object);
    max_note.serialize(root_object);
    transpose_delta.serialize(root_object);
    json_set_float_serialization_format("%.0f");
    serialized_string = json_serialize_to_string(root_value);
    json_value_free(root_value);
    dirty = false;
    return serialized_string;
}

bool rppicomidi::Midi_processor_transpose::deserialize_settings(const char* settings_str)
{
    JSON_Value *root_value = json_parse_string(settings_str);
    JSON_Object *root_object = NULL;
    bool result = false;
    if (root_value && json_value_get_type(root_value) == JSONObject) {
        root_object = json_value_get_object(root_value);

        if (chan.deserialize(root_object))
            result = true;

        if (!result || !min_note.deserialize(root_object))
            result = false;
 
        if (!result || !max_note.deserialize(root_object))
            result = false;

        if (!result || !transpose_delta.deserialize(root_object))
            result = false;
    }

    if (root_value)
        json_value_free(root_value);
    if (result)
        dirty = false;
    return result;
}

void rppicomidi::Midi_processor_transpose::load_defaults()
{
    chan.set_default();
    min_note.set_default();
    max_note.set_default();
    transpose_delta.set_default();
}
