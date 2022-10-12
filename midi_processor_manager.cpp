/* MIT License
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
#include "midi_processor_manager.h"
#include "midi_processor_mc_fader_pickup.h"
#include "midi_processor_transpose.h"
#include "midi_processor_chan_mes_remap.h"
#include "midi_processor_mc_fader_pickup_settings_view.h"
#include "midi_processor_transpose_view.h"
#include "midi_processor_chan_mes_remap_settings_view.h"

uint16_t rppicomidi::Midi_processor_manager::unique_id = 0;
rppicomidi::Midi_processor_manager::Midi_processor_manager() : screen{nullptr}
{
    // Note: try to add new processor types to this list alphabetically
    mutex_init(&processing_mutex);
    proclist.push_back({Midi_processor_mc_fader_pickup::static_getname(), Midi_processor_mc_fader_pickup::static_make_new,
                        Midi_processor_mc_fader_pickup_settings_view::static_make_new});
    proclist.push_back({Midi_processor_transpose::static_getname(), Midi_processor_transpose::static_make_new,
                        Midi_processor_transpose_view::static_make_new});
    proclist.push_back({Midi_processor_chan_mes_remap::static_getname(), Midi_processor_chan_mes_remap::static_make_new,
                        Midi_processor_chan_mes_remap_settings_view::static_make_new});
}

void rppicomidi::Midi_processor_manager::set_connected_device(uint16_t vid_, uint16_t pid_, uint8_t num_in_cables_, uint8_t num_out_cables_)
{
    // create data structures for managing processors for MIDI IN and MIDI OUT
    for (int cable = 0; cable < num_in_cables_; cable++) {
        midi_in_processors.push_back(std::vector<Mpv_element>());
        midi_in_proc_fns.push_back(std::vector<Midi_processor_fn>());
    }
    for (int cable = 0; cable < num_out_cables_; cable++) {
        midi_out_processors.push_back(std::vector<Mpv_element>());
        midi_out_proc_fns.push_back(std::vector<Midi_processor_fn>());
    }
    // Get stored settings for this device if any
    settings_file.set_vid_pid(vid_, pid_);
    if (!settings_file.load()) {
        printf("error loading settings for device %04x-%04x\r\n", vid_, pid_);
    }
}


size_t rppicomidi::Midi_processor_manager::get_midi_processor_idx_by_name(const char* name)
{
    size_t idx = name ? 0 : get_num_midi_processor_types(); // make sure name is not nullptr
    for (; idx < get_num_midi_processor_types() && strcmp(proclist[idx].name, name) != 0; idx++) {
    }
    return idx;
}

rppicomidi::Midi_processor_settings_view* rppicomidi::Midi_processor_manager::add_new_midi_processor_by_idx(size_t idx, uint8_t cable, bool is_midi_in)
{
    Midi_processor_settings_view* retview = nullptr;
    assert(screen);
    if (idx < proclist.size()) {
        auto proc = proclist[idx].processor(unique_id++);
        auto view = proclist[idx].view(*screen, screen->get_clip_rect(), proc);
        mutex_enter_blocking(&processing_mutex);
        if (is_midi_in) {
            midi_in_processors[cable].push_back({proc, view});
        }
        else {
            midi_out_processors[cable].push_back({proc, view});
        }
        build_processor_structures();
        mutex_exit(&processing_mutex);
        retview = view;
    }
    return retview;
}

void rppicomidi::Midi_processor_manager::delete_midi_processor_by_idx(int idx, uint8_t cable, bool is_midi_in)
{
    if (idx < 0) {
        printf("illegal idx=%d\r\n", idx);
        return; // invalid
    }
    mutex_enter_blocking(&processing_mutex);
    if (is_midi_in) {
        if (cable < midi_in_processors.size()) {
            if (idx < static_cast<int>(midi_in_processors[cable].size())) {
                auto it = midi_in_processors[cable].begin()+idx;
                delete (*it).proc;
                delete (*it).view;
                midi_in_processors[cable].erase(it);
                build_processor_structures();
            }
        }
    }
    else {
        if (cable < midi_out_processors.size()) {
            if (idx < static_cast<int>(midi_out_processors[cable].size())) {
                auto it = midi_out_processors[cable].begin()+idx;
                delete (*it).proc;
                delete (*it).view;
                midi_out_processors[cable].erase(it);
                build_processor_structures();
            }
        }
    }
    mutex_exit(&processing_mutex);
}

#if 0
void rppicomidi::Midi_processor_manager::delete_midi_processor_by_unique_id(uint16_t uid, uint8_t cable, bool is_midi_in)
{
    if (is_midi_in) {
        mutex_enter_blocking(&processing_mutex);
        if (cable < midi_in_processors.size()) {
            auto it = midi_in_processors[cable].begin();
            for (; it != midi_in_processors[cable].end(); it++) {
                if ((*it)->get_unique_id() == uid) {
                    delete (*it);
                    (*it) = nullptr;
                    break;
                }
            }
            if (it != midi_in_processors[cable].end()) {
                midi_in_processors[cable].erase(it);
                build_processor_structures();
            }
            else {
                printf("Processor id %u on MIDI %s cable %u not deleted\r\n", uid, is_midi_in ? "IN":"OUT", cable);
            }
        }
        mutex_exit(&processing_mutex);
    }
}
#endif

void rppicomidi::Midi_processor_manager::build_processor_structures()
{
    // erase all data structures associated with the processor lists
    for (size_t cable=0; cable < midi_in_processors.size(); cable++) {
        midi_in_proc_fns[cable].clear();
    }
    for (size_t cable=0; cable < midi_out_processors.size(); cable++) {
        midi_out_proc_fns[cable].clear();
    }
    processors_with_tasks.clear();

    // for each MIDI IN cable, add access to the process() method to the
    // MIDI IN processor function list for the cable #. If necessary,
    // add acess to the feedback() method to the MIDI OUT processor 
    // function list for the cable # and add the task() method the
    // background task list.
    for (size_t cable=0; cable < midi_in_processors.size(); cable++) {
        for (auto& midi_in_proc: midi_in_processors[cable]) {
            midi_in_proc_fns[cable].push_back(Midi_processor_fn{midi_in_proc.proc, false});
            if (midi_in_proc.proc->has_feedback_process() && cable < midi_out_proc_fns.size()) {
                midi_out_proc_fns[cable].push_back(Midi_processor_fn{midi_in_proc.proc, true});
            }
            if (midi_in_proc.proc->has_task()) {
                processors_with_tasks.push_back(midi_in_proc.proc);
            }
            midi_in_proc.proc->set_not_saved();
        }
    }

    // for each MIDI OUT cable, add access to the process() method to the
    // MIDI OUT processor function list for the cable #. If necessary,
    // add acess to the feedback() method to the MIDI IN processor 
    // function list for the cable # and add the task() method the
    // background task list.
    for (size_t cable=0; cable < midi_out_processors.size(); cable++) {
        for (auto& midi_out_proc: midi_out_processors[cable]) {
            midi_out_proc_fns[cable].push_back(Midi_processor_fn{midi_out_proc.proc, false});
            if (midi_out_proc.proc->has_feedback_process() && cable < midi_in_proc_fns.size()) {
                midi_in_proc_fns[cable].push_back(Midi_processor_fn{midi_out_proc.proc, true});
            }
            if (midi_out_proc.proc->has_task()) {
                processors_with_tasks.push_back(midi_out_proc.proc);
            }
            midi_out_proc.proc->set_not_saved();
        }
    }
}

bool rppicomidi::Midi_processor_manager::filter_midi_in(uint8_t cable, uint8_t* packet)
{
    bool donotfilter = true;
    //uint8_t cable = Midi_processor::get_cable_num(packet);
    mutex_enter_blocking(&processing_mutex);
    if (midi_in_proc_fns.size() > cable) {
        for (auto& process: midi_in_proc_fns[cable]) {
            if (process.is_feedback)
                donotfilter = process.proc->feedback(packet);
            else
                donotfilter = process.proc->process(packet);
            if (!donotfilter)
                break;
        }
    }
    mutex_exit(&processing_mutex);
    return donotfilter;
}


bool rppicomidi::Midi_processor_manager::filter_midi_out(uint8_t cable, uint8_t* packet)
{
    bool donotfilter = true;
    //uint8_t cable = Midi_processor::get_cable_num(packet);
    mutex_enter_blocking(&processing_mutex);
    if (midi_out_proc_fns.size() > cable) {
        for (auto& process: midi_out_proc_fns[cable]) {
            if (process.is_feedback)
                donotfilter = process.proc->feedback(packet);
            else
                donotfilter = process.proc->process(packet);
            if (!donotfilter)
                break;
        }
    }
    mutex_exit(&processing_mutex);
    return donotfilter;
}

void rppicomidi::Midi_processor_manager::task()
{
    static absolute_time_t previous_timestamp = {0};
    mutex_enter_blocking(&processing_mutex);
    for (auto& proc: processors_with_tasks) {
        proc->task();
    }

    // check if any processor settings are dirty once every 10 seconds
    // and if any are store the settings to flash
    absolute_time_t now = get_absolute_time();
    bool needs_store = false;
    
    int64_t diff = absolute_time_diff_us(previous_timestamp, now);
    if (diff > 60000000ll) { // check needs saving once per minute
        previous_timestamp = now;
        for (auto in_cable = midi_in_processors.begin(); !needs_store && in_cable != midi_in_processors.end(); in_cable++) {
            for (auto proc = in_cable->begin(); !needs_store && proc != in_cable->end(); proc++) {
                printf("processor %s needs store=%s\r\n",proc->proc->get_name(), proc->proc->not_saved() ? "True":"False");
                needs_store = proc->proc->not_saved();
            }
        }
        for (auto out_cable = midi_out_processors.begin(); !needs_store && out_cable != midi_out_processors.end(); out_cable++) {
            for (auto proc = out_cable->begin(); !needs_store && proc != out_cable->end(); proc++) {
                printf("processor %s needs store=%s\r\n",proc->proc->get_name(), proc->proc->not_saved() ? "True":"False");
                needs_store = proc->proc->not_saved();
            }
        }
    }
    mutex_exit(&processing_mutex);
    if (needs_store) {
        printf("storing all settings to flash\r\n");
        settings_file.store();
    }
 }

char* rppicomidi::Midi_processor_manager::serialize()
{
    mutex_enter_blocking(&processing_mutex); // Don't allow processing or changes while serializing
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    int idx = 1;
    for (auto& midi_in_cable_processors: midi_in_processors) {
        JSON_Value *midi_value = json_value_init_object();
        JSON_Object *midi_object = json_value_get_object(midi_value);
        for (auto& proc: midi_in_cable_processors) {
            proc.proc->serialize_settings(proc.proc->get_name(), midi_object);
        }
        std::string midi_name = std::string{"MIDI IN"}+std::to_string(idx++);
        json_object_set_value(root_object, midi_name.c_str(), midi_value);
    }
    idx = 1;
    for (auto& midi_out_cable_processors: midi_out_processors) {
        JSON_Value *midi_value = json_value_init_object();
        JSON_Object *midi_object = json_value_get_object(midi_value);
        for (auto& proc: midi_out_cable_processors) {
            proc.proc->serialize_settings(proc.proc->get_name(), midi_object);
        }
        std::string midi_name = std::string{"MIDI OUT"}+std::to_string(idx++);
        json_object_set_value(root_object, midi_name.c_str(), midi_value);
    }
    mutex_exit(&processing_mutex);
    json_set_float_serialization_format("%.0f");
    char* serialized_string = json_serialize_to_string(root_value);
    json_value_free(root_value);

    return serialized_string;
}

bool rppicomidi::Midi_processor_manager::deserialize(char* json_format)
{
    JSON_Value* root_value= json_parse_string(json_format);
    JSON_Object *root_object = NULL;
    bool result = true;
    if (root_value && json_value_get_type(root_value) == JSONObject) {
        root_object = json_value_get_object(root_value);
        size_t nobjects = json_object_get_count(root_object);
        const size_t nmidi_in = midi_in_processors.size(); 
        const size_t nmidi_out = midi_out_processors.size(); 
        // There should be as many objects as there are MIDI INs and MIDI OUTs
        if (nobjects == (nmidi_in + nmidi_out)) {
            printf("deserialize: got %u objects as expected\r\n", nobjects);
            for (size_t idx=0; result && idx < nobjects; idx++) {
                const char* label = json_object_get_name(root_object, idx);
                if (label == nullptr || strlen(label) < 8) {
                    result = false;
                    break; // shortest label will be MIDI IN?
                }
                const char* ptr = strstr(label, "MIDI IN");
                if (ptr != nullptr) {
                    size_t midi_in_port = atoi(ptr+7) - 1;
                    if (midi_in_port < nmidi_in) {
                        // Deserialize processors for the specified MIDI IN port
                        JSON_Value* proc_values = json_object_get_value_at(root_object, idx);
                        JSON_Object* proc_objects = json_value_get_object(proc_values);
                        size_t nproc_objects = json_object_get_count(proc_objects);
                        printf("%u processor objects in MIDI IN%u\r\n", nproc_objects, midi_in_port+1);

                        mutex_enter_blocking(&processing_mutex); // Don't allow processing while messing with vectors
                        midi_in_proc_fns[midi_in_port].clear();
                        midi_in_processors[midi_in_port].clear();
                        mutex_exit(&processing_mutex);

                        for (size_t proc_idx=0; result && (proc_idx < nproc_objects); proc_idx++) {
                            const char* proc_label = json_object_get_name(proc_objects, proc_idx);
                            size_t proc_type_idx = get_midi_processor_idx_by_name(proc_label);
                            if (proc_type_idx >= get_num_midi_processor_types() ) {
                                printf("deserialize: new processor name %s not found\r\n", proc_label);
                                result = false;
                            }
                            else {
                                add_new_midi_processor_by_idx(proc_type_idx, midi_in_port, true);
                                JSON_Value* setting_values = json_object_get_value_at(proc_objects, proc_idx);
                                JSON_Object* setting_objects = json_value_get_object(setting_values);
                                result = midi_in_processors[midi_in_port][proc_idx].proc->deserialize_settings(setting_objects);
                                if (!result) {
                                    printf("deserialize: failed to deserialize settings for %s\r\n", proc_label);
                                }
                            }
                        }
                    }
                    else {
                        printf("deserialize: bad MIDI port number %u\r\n", midi_in_port);
                        result = false;
                        break;
                    }
                }
            }
        }
        else {
            printf("deserialize: error got %u objects\r\n", nobjects);
        }
    }
    return result;
}