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
#include "midi_filter.h"
#include "class/midi/midi.h"
#include "midi_mc_fader_pickup.h"

#define KEYLAB_ESSENTIAL_NFADERS 9

// Assume that if abs(hardware fader value - daw fader value) is within 127, then the faders are synchronized
#define KEYLAB_ESSENTIAL_FADERS_DELTA 0x7f
static mc_fader_pickup_t fader_pickup[KEYLAB_ESSENTIAL_NFADERS]; // fader channels 1-8 plus the main fader

void filter_midi_init(void)
{
  for (int chan = 0; chan < KEYLAB_ESSENTIAL_NFADERS; chan++)
  {
    mc_fader_pickup_init(fader_pickup+chan, KEYLAB_ESSENTIAL_FADERS_DELTA);
  }
}

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

#if 0
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
#endif

// Filter messages from the Arturia Keylab Essential
bool filter_midi_in(uint8_t packet[4])
{
  bool packet_not_filtered_out = true;
  if (get_cable_num(packet) == 1)
  {
    // remap the note numbers for certain button presses
    if (packet[1] == 0x90 || packet[1] == 0x80)
    {
      switch (packet[2])
      {
        default:
          break;
        case 0x50: // Save button
          packet[2] = 0x48;
          break;
        case 0x51: // Undo button
          packet[2] = 0x46;
          break;
        case 0x58:
          packet_not_filtered_out = false;
          break;
      }
    }
    else if (packet[1] >= 0xe0 && packet[1] <= 0xe8)
    {
      // fader move from the Keylab Essential. Filter it out if the fader is not in sync with the DAW
      TU_LOG2("received packet %02x %02x %02x\r\n", packet[1], packet[2], packet[3]);
      packet_not_filtered_out = mc_fader_pickup_set_hw_fader_value(&fader_pickup[packet[1] & 0xf], mc_fader_extract_value(packet));
    }
  }
  return packet_not_filtered_out;
}

// Filter messages from the DAW
bool filter_midi_out(uint8_t packet[4])
{
  bool packet_not_filtered_out = true;
  if (get_cable_num(packet) == 1)
  {
    // remap the note numbers for certain button LEDs
    if (packet[1] == 0x90 || packet[1] == 0x80)
    {
      switch (packet[2])
      {
        default:
            break;
        case 0x48: // Save button
            packet[2] = 0x50;
            break;
        case 0x46: // Undo button
            packet[2] = 0x51;
            break;
      }
    }
    else if (packet[1] >= 0xe0 && packet[1] <= 0xe8)
    {
      // fader move command from DAW. Update Mackie Control fader synchronization
      packet_not_filtered_out = false;
      (void)mc_fader_pickup_set_daw_fader_value(&fader_pickup[packet[1] & 0xf], mc_fader_extract_value(packet));
    }
  }
  return packet_not_filtered_out;
}
