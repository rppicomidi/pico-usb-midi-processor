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

/**
 * @file midi_mc_fader_pickup.h
 * 
 * This file contains functions and data that implement fader "pickup" synchronization
 * between a DAW that expects to be connected to a Mackie Control control surface that
 * has motorized faders and a Mackie Control compatible control surface that has
 * non-motorized faders. The Mackie control fader move message is the MIDI pitch
 * bend message; pitch bend MIDI channels 1-8 correspond to fader channels 1-8;
 * pitch bend MIDI channel 9 corresponds to the main fader. Each fader value is a 14-bit
 * unsigned value encoded in the pitch bend position, 7 bit data, LSB first.
 * 
 * The DAW will send the current expected fader values to the control surface on
 * startup, fader bank change, mode change, etc. The faders on a control surface with
 * motorized faders will move to match the target fader values from the DAW. Non-motorized
 * faders cannot move to match target values all by themselves. This code provides data structures
 * that allow higher level code to filter out fader move messages to the DAW for an individual fader
 * until the user moves the fader up to or past the last target fader value the DAW sent for that fader.
 *
 * To use this code:
 * 1. Create as many mc_fader_pickup_t structures as you have hardware faders on your control surface
 * 2. Call mc_fader_pickup_init() to initialize each structure
 * 3. For each Mackie Control fader move message (MIDI pitch bend) you receive, extract the fader value
 *    by calling mc_fader_extract_value().
 * 4a. If the fader move message was from the DAW, update the structure by calling mc_fader_pickup_set_daw_fader_value().
 * Do not send the message to the control surface (it can't do anything with it anyway)
 * 4b. If the fader move message was from the hardware control surface, update the structure by calling
 * mc_fader_pickup_set_hw_fader_value() and note the return value. If the return value value was true, send
 * the fader move message to the DAW because the fader is in sync with the value the DAW thinks it should have.
 * Otherwise, do not send the fader move message to the DAW.
 */
#pragma once
#include <cstdint>
#include "midi_processor.h"

namespace rppicomidi{

class Mc_fader_pickup
{
public:
    /**
     * @brief Construct a new Mc_fader_pickup object
     * 
     */
    Mc_fader_pickup() : state{MC_FADER_PICKUP_RESET}, daw{0}, fader{0}, sync_delta{0x7f}
    {
    }

    /**
     * @brief initialize a mc_fader_pickup structure
     * 
     * @param sync_delta_ is the unsigned 14-bit absolute fader value difference 
     * that is close enough to call the hardware fader in sync with the DAW
     */
    void init(uint16_t sync_delta_);

    /**
     * @brief get the 14-bit unsigned fader value from the pitch bend USB MIDI packet
     * 
     * @param packet a standard 4-byte USB MIDI pitch bend message
     * @return uint16_t the 14-bit unsigned fader value
     *
     * @note this function ignores the first two bytes of the packet
     */
    uint16_t extract_fader_value(uint8_t packet[4]);

    /**
     * @brief encode the 14-bit unsigned fader value to the pitch bend USB MIDI packet
     *
     * @param fader_value the 14-bit fader value to encode in the pitch bend message
     * @param packet the standard USB MIDI pitch bend message with the fader value enocded
     *
     * @note this function does not set the the first two bytes of the packet
     */
    void encode_value(uint16_t fader_value, uint8_t packet[4]);

    /**
     * @brief write a new DAW fader value to the pickup structure
     *
     * @param daw_fader_value the 14-bit unsigned DAW fader value
     * @return true if setting the DAW fader causes the hardware fader and DAW fader to be in sync
     */   
    bool set_daw_fader_value(uint16_t daw_fader_value);

    /**
     * @brief write a new hardare fader value to the pickup structure
     * 
     * @param hw_fader_value the 14-bit unsigned hardware fader value
     * @return true if setting the hardware fader causes the hardware fader and DAW fader to be in sync
     */
    bool set_hw_fader_value(uint16_t hw_fader_value);
private:
    bool is_synchronized() { return state == MC_FADER_PICKUP_SYNCED; }
    enum state_e {
        MC_FADER_PICKUP_RESET,        //!< Fader state and the fader target value is unknown
        MC_FADER_PICKUP_HW_UNKNOWN,   //!< Hardware Fader value is unknown but the DAW value is known
        MC_FADER_PICKUP_DAW_UNKNOWN,  //!< Hardware Fader value is known but the DAW value is unknown
        MC_FADER_PICKUP_TOO_HIGH,     //!< Fader is too high; need to reduce it down to or less than the fader target value
        MC_FADER_PICKUP_TOO_LOW,      //!< Fader is too low; need to increase it up to or greater than the fader target value
        MC_FADER_PICKUP_SYNCED
    } state;  // the current pickup state of the fader
    uint16_t daw;                   // the last fader value the DAW sent (14-bits, unsigned)
    uint16_t fader;                 // the last fader value the control surface sent (14-bits, unsigned)
    uint16_t sync_delta;            // the minimum difference between the fader values before they are considered "equal" (14-bits, unsigned)
};


class Midi_processor_mc_fader_pickup : public Midi_processor
{
public:
    Midi_processor_mc_fader_pickup(uint16_t unique_id_, uint16_t sync_delta_=0x7f) : Midi_processor(static_getname(), unique_id_)
    {
        for (int fader=0; fader< num_faders; fader++)
            faders[fader].init(sync_delta_);
    }
    virtual ~Midi_processor_mc_fader_pickup()=default;
    bool process(uint8_t* packet) final;
    bool feedback(uint8_t* packet) final;
    bool has_feedback_process() final {return true; }
    // The following are manditory static methods to enable the Midi_processor_manager class
    static const char* static_getname() { return "MC Fader Pickup"; }
    static Midi_processor* static_make_new(uint16_t unique_id_) { return new Midi_processor_mc_fader_pickup(unique_id_); }
private:
    static const uint8_t num_faders=9;
    Mc_fader_pickup faders[num_faders];
};
}