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

#include "midi_mc_fader_pickup.h"
#include <stdio.h>
void mc_fader_pickup_init(mc_fader_pickup_t* pickup, uint16_t sync_delta)
{
  pickup->state = MC_FADER_PICKUP_RESET;
  pickup->sync_delta = sync_delta;
  pickup->daw = 0;
  pickup->fader = 0; 
}

uint16_t mc_fader_extract_value(uint8_t packet[4])
{
  return ((uint16_t)packet[2] & 0x7f) | ((uint16_t)(packet[3] & 0x7f) << 7);
}

void mc_fader_encode_value(uint16_t fader_value, uint8_t packet[4])
{
  packet[2] = fader_value & 0x7f;
  packet[3] = (fader_value >> 7) & 0x7f;
}

static bool mc_fader_state_is_synchronized(mc_fader_pickup_state_t state)
{
  return state == MC_FADER_PICKUP_SYNCED;
}

bool mc_fader_pickup_set_daw_fader_value(mc_fader_pickup_t* pickup, uint16_t daw_fader_value)
{
  mc_fader_pickup_state_t next_state = pickup->state; // assume state will not change
  int16_t delta = (int16_t)daw_fader_value - (int16_t)pickup->fader;
  pickup->daw = daw_fader_value;
  switch(pickup->state)
  {
    case MC_FADER_PICKUP_RESET:
    case MC_FADER_PICKUP_HW_UNKNOWN:
      next_state = MC_FADER_PICKUP_HW_UNKNOWN;
      break;

    default: // both DAW fader value and Hardware fader value are known
    {
      uint16_t abs_delta = delta;
      if (delta < 0)
        abs_delta = -delta;
      if (abs_delta < pickup->sync_delta)
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
  pickup->state = next_state;
  return mc_fader_state_is_synchronized(next_state);
}

bool mc_fader_pickup_set_hw_fader_value(mc_fader_pickup_t* pickup, uint16_t hw_fader_value)
{
  int16_t delta = (int16_t)hw_fader_value - (int16_t)pickup->daw;
  uint16_t abs_delta = delta;
  mc_fader_pickup_state_t next_state = pickup->state; // assume state will not change
  if (delta < 0)
  abs_delta = -delta;
  switch(pickup->state)
  {
    case MC_FADER_PICKUP_RESET:
    case MC_FADER_PICKUP_DAW_UNKNOWN:
      next_state = MC_FADER_PICKUP_DAW_UNKNOWN;
      break;
    case MC_FADER_PICKUP_SYNCED:
      break;
    case MC_FADER_PICKUP_TOO_HIGH:
      if (abs_delta < pickup->sync_delta)
        next_state = MC_FADER_PICKUP_SYNCED;
      else if (delta < 0) {
          // previous fader value was higher than the DAW fader value, and now is
          // lower, so the hardware fader must have moved past the DAW value
          next_state = MC_FADER_PICKUP_SYNCED;
      }
      // otherwise, still too high
      break;
    case MC_FADER_PICKUP_TOO_LOW:
      if (abs_delta < pickup->sync_delta)
        next_state = MC_FADER_PICKUP_SYNCED;
      else if (delta > 0) {
          // previous fader value was lower than the DAW fader value, and now is
          // higher, so the hardware fader must have moved past the DAW value
          next_state = MC_FADER_PICKUP_SYNCED;
      }
      // otherwise, still too low
      break;
    case MC_FADER_PICKUP_HW_UNKNOWN:
      if (abs_delta < pickup->sync_delta)
        next_state = MC_FADER_PICKUP_SYNCED;
      else if (delta > 0)
        next_state = MC_FADER_PICKUP_TOO_HIGH;
      else
        next_state = MC_FADER_PICKUP_TOO_LOW;
      break;
    default:
      printf("unknown pickup state %u\r\n", pickup->state);
      next_state = MC_FADER_PICKUP_RESET;
      break;
  }
  pickup->state = next_state;
  pickup->fader = hw_fader_value;
  return mc_fader_state_is_synchronized(next_state);
}

