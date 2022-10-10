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

void rppicomidi::Midi_processor_transpose::serialize_settings(const char* name, JSON_Object *root_object)
{
    JSON_Value *proc_value = json_value_init_object();
    JSON_Object *proc_object = json_value_get_object(proc_value);
    chan.serialize(proc_object);
    min_note.serialize(proc_object);
    max_note.serialize(proc_object);
    transpose_delta.serialize(proc_object);
    json_object_set_value(root_object, name, proc_value);
    dirty = false;
}

bool rppicomidi::Midi_processor_transpose::deserialize_settings(JSON_Object *root_object)
{
    bool result = false;
    if (chan.deserialize(root_object))
        result = true;

    if (!result || !min_note.deserialize(root_object))
        result = false;

    if (!result || !max_note.deserialize(root_object))
        result = false;

    if (!result || !transpose_delta.deserialize(root_object))
        result = false;
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
