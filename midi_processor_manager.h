/**
 * @file midi_processor_factory.h
 * @brief this class creates new Midi_processor objects by a processor
 * index number. It is designed to be used with a UI that allows selecting
 * a MIDI Processor stage by name from a list of names. Each UI that
 * shows the list of names calls get_num_midi_processor_types(); MIDI
 * Processor index numbers range 0-get_num_midi_processor_types()-1.
 * The UI builds a list of names by repeatedly calling get_midi_processor_name_by_idx().
 * To create a new Midi_processor object with a unique identifier number, call
 * get_new_midi_processor_by_idx(). It is the responsibility of the user of this class
 * to delete Midi_processor objects by calling delete <pointer to the object> if the
 * user of this class no longer needs the Midi_processor object that this class
 * created for it.
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
#pragma once
#include <vector>
#include "midi_processor.h"
#include "midi_processor_settings_view.h"
#include "pico/mutex.h"
#include "view.h"
#include "settings_file.h"

namespace rppicomidi
{
class Midi_processor_manager
{
private:
    typedef Midi_processor* (*mp_factory_fn)(uint16_t unique_id);
    typedef Midi_processor_settings_view* (*mpsv_factory_fn)(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor* proc_);
    /**
     * @brief Midi_processor ptr with a flag to choose whether to call the process() 
     * or feedback() function
     */
    struct Midi_processor_fn
    {   
        Midi_processor* proc;   //!< pointer to the Midi_processor whose process() or feedback() function is called
        bool is_feedback;       //!< if true, call proc->feedback(); otherwise call proc->process().
    };

public:
    // Singleton Pattern

    /**
     * @brief Get the Instance object
     *
     * @return the singleton instance
     */
    static Midi_processor_manager& instance()
    {
        static Midi_processor_manager _instance;    // Guaranteed to be destroyed.
                                                    // Instantiated on first use.
        return _instance;
    }
    Midi_processor_manager(Midi_processor_manager const&) = delete;
    void operator=(Midi_processor_manager const&) = delete;

    /**
     * @brief Construct a new Midi_processor_manager object
     */
    Midi_processor_manager();

    /**
     * @brief Get the number of MIDI Processor types
     * 
     * @return size_t the number of MIDI Processor types
     */
    size_t get_num_midi_processor_types() {return proclist.size(); }

    /**
     * @brief Get the midi processor name by idx
     * 
     * @param idx the processor number, from 0. Must be < get_num_midi_processor_types()
     * @return const char* nullptr if idx is out of range or a pointer to the MIDI Processor's name at index idx
     */
    const char* get_midi_processor_name_by_idx(size_t idx)
    {
        if (idx < proclist.size())
            return proclist[idx].name;
        return nullptr;
    }

    /**
     * @brief Get the midi processor idx by its name
     * 
     * @param name the processor name
     * @return get_num_midi_processor_types() if the name is not found, otherwise the index value
     */
    size_t get_midi_processor_idx_by_name(const char* name);

    /**
     * @brief Add the new MIDI processor created by proclist[idx].processor() to the end of
     * the processor list for the cable number and direction
     * 
     * @param idx specifies the processor with name proclist[idx].name
     * @param cable is the virtual cable number from 0
     * @param is_midi_in is true if the processor is for the MIDI IN direction, false for the OUT direction
     * @return a pointer to the newly added Midi_processor object
     */
    Midi_processor_settings_view* add_new_midi_processor_by_idx(size_t idx, uint8_t cable, bool is_midi_in);

    /**
     * @brief delete a MIDI processor based on its current index in the processing list
     * for the given MIDI cable number and direction
     * 
     * @param idx location in the midi_processors array
     * @param cable the USB virtual cable number from 0
     * @param is_midi_in true if direction is MIDI IN (from the connected device) or MIDI OUT (to the connected device)
     */
    void delete_midi_processor_by_idx(int idx, uint8_t cable, bool is_midi_in);

    int get_num_midi_processors(uint8_t cable, bool is_midi_in)
    {
        int retval = -1;
        if (is_midi_in && cable < midi_in_processors.size()) {
            retval = midi_in_processors[cable].size();
        }
        else if (!is_midi_in && cable < midi_out_processors.size()) {
            retval = midi_out_processors[cable].size();
        }
        return retval;
    }

    Midi_processor* get_midi_processor_by_index(size_t idx, uint8_t cable, bool is_midi_in)
    {
        Midi_processor* retval = nullptr;
        if (is_midi_in && cable < midi_in_processors.size()) {
            if (idx < midi_in_processors[cable].size()) {
                retval = midi_in_processors[cable][idx].proc;
            }
        }
        else if (!is_midi_in && cable < midi_out_processors.size()) {
            if (idx < midi_out_processors[cable].size()) {
                retval = midi_out_processors[cable][idx].proc;
            }
        }
        return retval;
    }

    Midi_processor_settings_view* get_midi_processor_view_by_index(size_t idx, uint8_t cable, bool is_midi_in)
    {
        Midi_processor_settings_view* retval = nullptr;
        if (is_midi_in && cable < midi_in_processors.size()) {
            if (idx < midi_in_processors[cable].size()) {
                retval = midi_in_processors[cable][idx].view;
            }
        }
        else if (!is_midi_in && cable < midi_out_processors.size()) {
            if (idx < midi_out_processors[cable].size()) {
                retval = midi_out_processors[cable][idx].view;
            }
        }
        return retval;
    }

    /**
     * @brief Initialize the processor lists for the newly connected device
     * 
     * @param vid_ the idVendor of the newly connected MIDI device
     * @param pid_ the idProduct of the newly connected MIDI device
     * @param num_in_cables_ the number of virtual MIDI IN cables
     * @param num_out_cables_ the number of virtual MIDI OUT cables
     */
    void set_connected_device(uint16_t vid_, uint16_t pid_, uint8_t num_in_cables_, uint8_t num_out_cables_);

    /**
     * @brief process the MIDI IN packet from the connected device on the specified virtual cable 
     * 
     * @param cable_ the USB MIDI virtual cable number. (yes, it is also in the packet; this is for future
     * when I may switch to streams instead of USB packets)
     * @param packet_ the 4-byte USB MIDI packet
     * @return true if the packet should be sent to the Pico's USB device interface
     * @return false if the packet should be discarded
     */
    bool filter_midi_in(uint8_t cable_, uint8_t* packet_);

    /**
     * @brief process the MIDI OUT packet sent to the connected device on the specified virtual cable 
     * 
     * @param cable_ the USB MIDI virtual cable number. (yes, it is also in the packet; this is for future
     * when I may switch to streams instead of USB packets)
     * @param packet_ the 4-byte USB MIDI packet
     * @return true if the packet should be sent to the connected MIDI Device
     * @return false if the packet should be discarded
     */
    bool filter_midi_out(uint8_t cable_, uint8_t* packet_);

    /**
     * @brief execute the task() functions for all Midi_processor objects
     * that have a task() function that does anything
     */
    void task();

    /**
     * @brief Set the screen object
     *
     * This function must be called before any MIDI processor settings view is created.
     *
     * @param screen_ a pointer to the screen that displays the settings
     */
    void set_screen(Mono_graphics* screen_) {screen = screen_;}

    /**
     * @brief serialize to a JSON-formatted string the settings
     * of all processors on all MIDI INs and MIDI OUTs
     * 
     * @return char* the serialized string
     */
    char* serialize();

    /**
     * @brief deserialize the JSON-formatted string to the settings
     * of all processors on all MIDI INs and MIDI OUTs. If a MIDI IN
     * or MIDI OUT does not have a processor corresponding to the settings,
     * allocate a new one first
     *
     * @param json_format 
     * @return true if deserialization was successful
     * @return false if deserialization failed
     */
    bool deserialize(char* json_format);
private:
    /**
     * @brief rebuild the midi_in_proc_fns midi_out_proc_fns and processors_with_tasks
     * vectors.
     *
     * Call this function after either midi_in_processors or midi_out_processors change.
     * Call this with the processing_mutex locked.
     */
    void build_processor_structures();

    struct Mpf_element {
        const char* name;
        mp_factory_fn processor;
        mpsv_factory_fn view;
    };
    std::vector<Mpf_element> proclist;
    static uint16_t unique_id;
    mutex processing_mutex;
    struct Mpv_element {
        Midi_processor* proc;
        Midi_processor_settings_view* view;
    };
    std::vector<std::vector<Mpv_element>> midi_in_processors;
    std::vector<std::vector<Mpv_element>> midi_out_processors;
    std::vector<std::vector<Midi_processor_fn>> midi_in_proc_fns;
    std::vector<std::vector<Midi_processor_fn>> midi_out_proc_fns;
    std::vector<Midi_processor*> processors_with_tasks;
    Mono_graphics* screen;
    Settings_file settings_file;
};
}