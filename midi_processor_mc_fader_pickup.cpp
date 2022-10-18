/*
 * The MIT License (MIT)
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "midi_processor_mc_fader_pickup.h"
#include "parson.h"
#include <stdio.h>
void rppicomidi::Mc_fader_pickup::init(uint16_t sync_delta_)
{
    state = MC_FADER_PICKUP_RESET;
    sync_delta = sync_delta_;
    daw = 0;
    fader = 0; 
}

uint16_t rppicomidi::Mc_fader_pickup::extract_fader_value(uint8_t packet[4])
{
  return ((uint16_t)packet[2] & 0x7f) | ((uint16_t)(packet[3] & 0x7f) << 7);
}

void rppicomidi::Mc_fader_pickup::encode_value(uint16_t fader_value, uint8_t packet[4])
{
    packet[2] = fader_value & 0x7f;
    packet[3] = (fader_value >> 7) & 0x7f;
}


bool rppicomidi::Mc_fader_pickup::set_daw_fader_value(uint16_t daw_fader_value)
{
    state_e next_state = state; // assume state will not change
    switch(state)
    {
        case MC_FADER_PICKUP_RESET:
        case MC_FADER_PICKUP_HW_UNKNOWN:
            next_state = MC_FADER_PICKUP_HW_UNKNOWN;
            break;

        default: // both DAW fader value and Hardware fader value are known
        {
            int16_t delta = (int16_t)daw_fader_value - (int16_t)fader;
            uint16_t abs_delta = delta;
            if (delta < 0)
                abs_delta = -delta;
            if (abs_delta < sync_delta)
            {
                next_state = MC_FADER_PICKUP_SYNCED;
            }
            else if (delta < 0)
            {
                next_state = MC_FADER_PICKUP_TOO_HIGH;
            }
            else
            {
                next_state = MC_FADER_PICKUP_TOO_LOW;
            }
        }
    }
    state = next_state;
    daw = daw_fader_value;
    return is_synchronized();
}

bool rppicomidi::Mc_fader_pickup::set_hw_fader_value(uint16_t hw_fader_value)
{
    int16_t delta = (int16_t)hw_fader_value - (int16_t)daw;
    uint16_t abs_delta = delta;
    state_e next_state = state; // assume state will not change
    if (delta < 0)
        abs_delta = -delta;
    switch(state)
    {
        case MC_FADER_PICKUP_RESET:
        case MC_FADER_PICKUP_DAW_UNKNOWN:
            next_state = MC_FADER_PICKUP_DAW_UNKNOWN;
            break;
        case MC_FADER_PICKUP_SYNCED:
            break;
        case MC_FADER_PICKUP_TOO_HIGH:
            if (abs_delta < sync_delta)
                next_state = MC_FADER_PICKUP_SYNCED;
            else if (delta < 0) {
                // previous fader value was higher than the DAW fader value, and now is
                // lower, so the hardware fader must have moved past the DAW value
                next_state = MC_FADER_PICKUP_SYNCED;
            }
            // otherwise, still too high
            break;
        case MC_FADER_PICKUP_TOO_LOW:
            if (abs_delta < sync_delta)
                next_state = MC_FADER_PICKUP_SYNCED;
            else if (delta > 0) {
                // previous fader value was lower than the DAW fader value, and now is
                // higher, so the hardware fader must have moved past the DAW value
                next_state = MC_FADER_PICKUP_SYNCED;
            }
            // otherwise, still too low
            break;
        case MC_FADER_PICKUP_HW_UNKNOWN:
            if (abs_delta < sync_delta)
                next_state = MC_FADER_PICKUP_SYNCED;
            else if (delta > 0)
                next_state = MC_FADER_PICKUP_TOO_HIGH;
            else
                next_state = MC_FADER_PICKUP_TOO_LOW;
            break;
        default:
            printf("unknown pickup state %u\r\n", state);
            next_state = MC_FADER_PICKUP_RESET;
            break;
    }
    state = next_state;
    fader = hw_fader_value;
    return is_synchronized();
}

bool rppicomidi::Midi_processor_mc_fader_pickup::process(uint8_t* packet)
{
    bool do_not_filter_out = true; // only filter out pitch bend messages if required
    if (packet[1]>= 0xE0 && packet[1] <= 0xE8) {
        // fader pitch bend message (the channel number determines the fader number)
        auto& fader  = faders[Midi_processor::get_channel_num(packet) - 1];
        do_not_filter_out = fader.set_hw_fader_value(fader.extract_fader_value(packet));
    }
    return do_not_filter_out;
}

bool rppicomidi::Midi_processor_mc_fader_pickup::feedback(uint8_t* packet)
{
     // passing the message along after processing does no harm;
     // if you are using this, your device probably ignores this packet
    bool do_not_filter_out = true;
    if (packet[1]>= 0xE0 && packet[1] <= 0xE8) {
        // fader pitch bend message
        auto& fader  = faders[Midi_processor::get_channel_num(packet) - 1];
        fader.set_daw_fader_value(fader.extract_fader_value(packet));
    }
    return do_not_filter_out;
}
