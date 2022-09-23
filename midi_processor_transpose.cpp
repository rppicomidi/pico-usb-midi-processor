#include "midi_processor_transpose.h"

bool rppicomidi::Midi_processor_transpose::process(uint8_t* packet)
 {
    bool success = true; // Only block passing the message on if transposing makes the note out of MIDI range
    if (get_channel_num(packet) == chan) {
        uint8_t status = packet[1] & 0xf0;
        if (status == 0x90 || status == 0x80) {
            // note message
            if (packet[2] >= min_note && packet[2] <= max_note) {
                int new_note = packet[2] + transpose_delta;
                if (new_note >= 0 && new_note <= 127) {
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