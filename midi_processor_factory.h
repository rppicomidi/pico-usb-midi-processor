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
#include "pico/mutex.h"
namespace rppicomidi
{
class Midi_processor_factory
{
public:
    typedef Midi_processor* (*factory_fn)(uint16_t unique_id);

public:
    // Singleton Pattern

    /**
     * @brief Get the Instance object
     *
     * @return the singleton instance
     */
    static Midi_processor_factory& instance()
    {
        static Midi_processor_factory _instance;    // Guaranteed to be destroyed.
                                                    // Instantiated on first use.
        return _instance;
    }
    Midi_processor_factory(Midi_processor_factory const&) = delete;
    void operator=(Midi_processor_factory const&) = delete;

    /**
     * @brief Construct a new Midi_processor_factory object
     */
    Midi_processor_factory();

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

    void add_new_midi_processor_by_idx(size_t idx, uint8_t cable, bool is_midi_in);

    void set_connected_device(uint16_t vid_, uint16_t pid_, uint8_t num_in_cables_, uint8_t num_out_cables_);

    bool filter_midi_in(uint8_t cable_, uint8_t* packet_);
    bool filter_midi_out(uint8_t cable_, uint8_t* packet_);
    void task();
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
        factory_fn processor;
    };
    std::vector<Mpf_element> proclist;
    static uint16_t unique_id;
    mutex processing_mutex;
    std::vector<std::vector<Midi_processor*>> midi_in_processors;
    std::vector<std::vector<Midi_processor*>> midi_out_processors;
    std::vector<std::vector<Midi_processor_fn>> midi_in_proc_fns;
    std::vector<std::vector<Midi_processor_fn>> midi_out_proc_fns;
    std::vector<Midi_processor*> processors_with_tasks;
};
}