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
#include "midi_processor_chan_button_remap.h"
#include "midi_processor_mc_fader_pickup_settings_view.h"
#include "midi_processor_transpose_view.h"
#include "midi_processor_chan_mes_remap_settings_view.h"

uint16_t rppicomidi::Midi_processor_manager::unique_id = 0;
rppicomidi::Midi_processor_manager::Midi_processor_manager() : screen{nullptr}, current_preset{"current preset",1,8,1}, dirty{true}
{
    // Note: try to add new processor types to this list alphabetically
    mutex_init(&processing_mutex);
    proclist.push_back({Midi_processor_mc_fader_pickup::static_getname(), Midi_processor_mc_fader_pickup::static_make_new,
                        Midi_processor_mc_fader_pickup_settings_view::static_make_new});
    proclist.push_back({Midi_processor_transpose::static_getname(), Midi_processor_transpose::static_make_new,
                        Midi_processor_transpose_view::static_make_new});
    proclist.push_back({Midi_processor_chan_mes_remap::static_getname(), Midi_processor_chan_mes_remap::static_make_new,
                        Midi_processor_chan_mes_remap_settings_view::static_make_new});
    proclist.push_back({Midi_processor_chan_button_remap::static_getname(), Midi_processor_chan_button_remap::static_make_new,
                        Midi_processor_chan_mes_remap_settings_view::static_make_new});
    *id_str = '\0';
    *prod_str = '\0';
    Settings_file::instance(); // construct the Settings_file instance
}

void rppicomidi::Midi_processor_manager::set_connected_device(uint16_t vid_, uint16_t pid_, const char* prod_str_, uint8_t num_in_cables_, uint8_t num_out_cables_)
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
    Settings_file::instance().set_vid_pid(vid_, pid_);
    if (!Settings_file::instance().load()) {
        printf("error loading settings for device %04x-%04x\r\n", vid_, pid_);
    }
    strncpy(prod_str, prod_str_, MAX_PROD_STR_NAME);
    Settings_file::instance().get_filename(id_str);
    prod_str[MAX_PROD_STR_NAME] = '\0';
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
        dirty = true;
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
    dirty = true;
    mutex_exit(&processing_mutex);
}

void rppicomidi::Midi_processor_manager::clear_all_processors()
{
    mutex_enter_blocking(&processing_mutex); // Don't allow processing while messing with vectors
    // erase all data structures associated with the processor lists
    for (size_t cable=0; cable < midi_in_processors.size(); cable++) {
        midi_in_proc_fns[cable].clear();
        for (auto& proc: midi_in_processors[cable]) {
            delete proc.proc;
            delete proc.view;
        }
        midi_in_processors[cable].clear();
    }
    for (size_t cable=0; cable < midi_out_processors.size(); cable++) {
        midi_out_proc_fns[cable].clear();
        for (auto& proc: midi_out_processors[cable]) {
            delete proc.proc;
            delete proc.view;
        }
        midi_out_processors[cable].clear();
    }
    processors_with_tasks.clear();
    mutex_exit(&processing_mutex);

}

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
    mutex_enter_blocking(&processing_mutex);
    for (auto& proc: processors_with_tasks) {
        proc->task();
    }
    mutex_exit(&processing_mutex);
 }


bool rppicomidi::Midi_processor_manager::needs_store()
{
    for (auto in_cable = midi_in_processors.begin(); !dirty && in_cable != midi_in_processors.end(); in_cable++) {
        for (auto proc = in_cable->begin(); !dirty && proc != in_cable->end(); proc++) {
            printf("processor %s needs store=%s\r\n",proc->proc->get_name(), proc->proc->not_saved() ? "True":"False");
            dirty = proc->proc->not_saved();
        }
    }
    for (auto out_cable = midi_out_processors.begin(); !dirty && out_cable != midi_out_processors.end(); out_cable++) {
        for (auto proc = out_cable->begin(); !dirty && proc != out_cable->end(); proc++) {
            printf("processor %s needs store=%s\r\n",proc->proc->get_name(), proc->proc->not_saved() ? "True":"False");
            dirty = proc->proc->not_saved();
        }
    }
    return dirty;
}

char* rppicomidi::Midi_processor_manager::serialize(uint8_t preset_num, char* previous_serialized_string)
{
    if (preset_num < current_preset.get_min() || preset_num > current_preset.get_max())
        return nullptr;
    bool free_previous = false;
    if (previous_serialized_string == nullptr) {
        previous_serialized_string = serialize_default();
        free_previous = true;
    }
    JSON_Value* root_value= json_parse_string(previous_serialized_string);
    if (free_previous) {
        json_free_serialized_string(previous_serialized_string);
    }
    if (root_value && json_value_get_type(root_value) == JSONObject) {
        mutex_enter_blocking(&processing_mutex); // Don't allow processing or changes while serializing
        JSON_Object* root_object = json_value_get_object(root_value);

        current_preset.set(preset_num);
        JSON_Value* preset_value = json_object_get_value(root_object, std::to_string(current_preset.get()).c_str());
        if (preset_value == nullptr) {
            // something is wrong with the previous_serialized_string
            char* default_json = serialize_default();
            if (default_json) {
                json_value_free(root_value);
                root_value= json_parse_string(default_json);
                json_free_serialized_string(default_json);
                if (root_value) {
                    root_object = json_value_get_object(root_value);
                    preset_value = json_object_get_value(root_object, std::to_string(current_preset.get()).c_str());
                }
            }
        }
        if (preset_value && json_value_get_type(preset_value) == JSONObject) {
            current_preset.serialize(root_object);
            JSON_Object* preset_object = json_value_get_object(preset_value);
            int idx = 1;
            for (auto& midi_in_cable_processors: midi_in_processors) {
                JSON_Value *midi_value = json_value_init_object();
                JSON_Object *midi_object = json_value_get_object(midi_value);
                for (auto& proc: midi_in_cable_processors) {
                    proc.proc->serialize_settings(proc.proc->get_unique_name(), midi_object);
                }
                std::string midi_name = std::string{"MIDI IN"}+std::to_string(idx++);
                json_object_set_value(preset_object, midi_name.c_str(), midi_value);
            }
            idx = 1;
            for (auto& midi_out_cable_processors: midi_out_processors) {
                JSON_Value *midi_value = json_value_init_object();
                JSON_Object *midi_object = json_value_get_object(midi_value);
                for (auto& proc: midi_out_cable_processors) {
                    proc.proc->serialize_settings(proc.proc->get_unique_name(), midi_object);
                }
                std::string midi_name = std::string{"MIDI OUT"}+std::to_string(idx++);
                json_object_set_value(preset_object, midi_name.c_str(), midi_value);
            }
        }
    }
    dirty = false;
    mutex_exit(&processing_mutex);
    json_set_float_serialization_format("%.0f");
    char* serialized_string = json_serialize_to_string(root_value);
    json_value_free(root_value);
    return serialized_string;
}

char* rppicomidi::Midi_processor_manager::serialize_default()
{
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "id", id_str);
    json_object_set_string(root_object, "prod", prod_str);


    current_preset.serialize(root_object);
    for (uint8_t preset_num = current_preset.get_min(); preset_num <= current_preset.get_max(); preset_num++) {
        int idx = 1;
        JSON_Value* preset_value = json_value_init_object();
        JSON_Object *preset_object = json_value_get_object(preset_value);
        for (auto midi_in_cable_processors: midi_in_processors) {
            JSON_Value *midi_value = json_value_init_object();
            std::string midi_name = std::string{"MIDI IN"}+std::to_string(idx++);
            json_object_set_value(preset_object, midi_name.c_str(), midi_value);
        }
        idx = 1;
        for (auto midi_out_cable_processors: midi_out_processors) {
            JSON_Value *midi_value = json_value_init_object();
            std::string midi_name = std::string{"MIDI OUT"}+std::to_string(idx++);
            json_object_set_value(preset_object, midi_name.c_str(), midi_value);
        }
        json_object_set_value(root_object, std::to_string(preset_num).c_str(), json_object_get_wrapping_value(preset_object));
    }
    json_set_float_serialization_format("%.0f");
    char* serialized_string = json_serialize_to_string(root_value);
    json_value_free(root_value);

    return serialized_string;

}

bool rppicomidi::Midi_processor_manager::deserialize_preset(uint8_t preset_num, char* json_format)
{
    JSON_Value* root_value= json_parse_string(json_format);
    JSON_Object *root_object = NULL;
    bool result = true;
    if (root_value && json_value_get_type(root_value) == JSONObject) {
        root_object = json_value_get_object(root_value);
        uint8_t last_preset = json_object_get_number(root_object, current_preset.get_name());
        // make sure the JSON formatted string to deserialize has current_preset set to preset_num
        if (last_preset != preset_num) {
            json_object_set_number(root_object, current_preset.get_name(), preset_num);
            json_set_float_serialization_format("%.0f");
            char* serialized_string = json_serialize_to_string(root_value);
            result = deserialize(serialized_string);
            json_free_serialized_string(serialized_string);
        }
        else {
            result = deserialize(json_format);
        }
    }
    else {
        result = false;
    }
    if (root_value)
        json_value_free(root_value);
    if (result)
        dirty = false;
    return result;
}

bool rppicomidi::Midi_processor_manager::deserialize(char* json_format)
{
    if (!json_format)
        return false;
    JSON_Value* root_value= json_parse_string(json_format);
    JSON_Object *root_object = NULL;
    bool result = true;
    if (root_value && json_value_get_type(root_value) == JSONObject) {
        root_object = json_value_get_object(root_value);
        // get the last preset number
        current_preset.deserialize(root_object);
        uint8_t last_preset = json_object_get_number(root_object, current_preset.get_name());
        if (last_preset >= current_preset.get_min() && last_preset <= current_preset.get_max()) {
            JSON_Object* preset = json_object_get_object(root_object, std::to_string(last_preset).c_str());
            size_t nobjects = json_object_get_count(preset);
            const size_t nmidi_in = midi_in_processors.size(); 
            const size_t nmidi_out = midi_out_processors.size();
            // There should be as many objects as there are MIDI INs and MIDI OUTs
            if (nobjects == (nmidi_in + nmidi_out)) {
                printf("deserialize: got %u objects as expected\r\n", nobjects);
                // clear out the existing data
                clear_all_processors();
                // deserialize all processors to all cables in both directions
                for (size_t idx=0; result && idx < nobjects; idx++) {
                    const char* label = json_object_get_name(preset, idx);
                    if (label == nullptr || strlen(label) < 8) {
                        result = false;
                        break; // shortest label will be MIDI IN?
                    }
                    const char* ptr = strstr(label, "MIDI IN");
                    if (ptr != nullptr) {
                        size_t midi_in_port = atoi(ptr+7) - 1;
                        if (midi_in_port < nmidi_in) {
                            // Deserialize processors for the specified MIDI IN port
                            JSON_Value* proc_values = json_object_get_value_at(preset, idx);
                            JSON_Object* proc_objects = json_value_get_object(proc_values);
                            size_t nproc_objects = json_object_get_count(proc_objects);
                            printf("%u processor objects in MIDI IN%u\r\n", nproc_objects, midi_in_port+1);

                            for (size_t proc_idx=0; result && (proc_idx < nproc_objects); proc_idx++) {
                                const char* proc_label = json_object_get_name(proc_objects, proc_idx);
                                const size_t proc_label_len = strlen(proc_label);
                                assert(proc_label_len > 0);
                                char proc_type_label[proc_label_len];
                                strcpy(proc_type_label, proc_label);
                                char* dollar_ptr = strrchr(proc_type_label,'$');
                                if (dollar_ptr != nullptr)
                                    *dollar_ptr = '\0';
                                size_t proc_type_idx = get_midi_processor_idx_by_name(proc_type_label);
                                if (proc_type_idx >= get_num_midi_processor_types() ) {
                                    printf("deserialize: new processor name %s not found\r\n", proc_label);
                                    result = false;
                                    break;
                                }
                                else {
                                    add_new_midi_processor_by_idx(proc_type_idx, midi_in_port, true);
                                    JSON_Value* setting_values = json_object_get_value_at(proc_objects, proc_idx);
                                    JSON_Object* setting_objects = json_value_get_object(setting_values);
                                    result = midi_in_processors[midi_in_port][proc_idx].proc->deserialize_settings(setting_objects);
                                    if (!result) {
                                        printf("deserialize: failed to deserialize settings for %s\r\n", proc_type_label);
                                        break;
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
                    ptr = strstr(label, "MIDI OUT");
                    if (ptr != nullptr) {
                        size_t midi_out_port = atoi(ptr+8) - 1;
                        if (midi_out_port < nmidi_out) {
                            // Deserialize processors for the specified MIDI IN port
                            JSON_Value* proc_values = json_object_get_value_at(preset, idx);
                            JSON_Object* proc_objects = json_value_get_object(proc_values);
                            size_t nproc_objects = json_object_get_count(proc_objects);
                            printf("%u processor objects in MIDI OUT%u\r\n", nproc_objects, midi_out_port+1);

                            for (size_t proc_idx=0; result && (proc_idx < nproc_objects); proc_idx++) {
                                const char* proc_label = json_object_get_name(proc_objects, proc_idx);
                                const size_t proc_label_len = strlen(proc_label);
                                assert(proc_label_len > 0);
                                char proc_type_label[proc_label_len];
                                strcpy(proc_type_label, proc_label);
                                char* dollar_ptr = strrchr(proc_type_label,'$');
                                if (dollar_ptr != nullptr)
                                    *dollar_ptr = '\0';
                                size_t proc_type_idx = get_midi_processor_idx_by_name(proc_type_label);
                                if (proc_type_idx >= get_num_midi_processor_types() ) {
                                    printf("deserialize: new processor name %s not found\r\n", proc_type_label);
                                    result = false;
                                    break;
                                }
                                else {
                                    add_new_midi_processor_by_idx(proc_type_idx, midi_out_port, true);
                                    JSON_Value* setting_values = json_object_get_value_at(proc_objects, proc_idx);
                                    JSON_Object* setting_objects = json_value_get_object(setting_values);
                                    result = midi_out_processors[midi_out_port][proc_idx].proc->deserialize_settings(setting_objects);
                                    if (!result) {
                                        printf("deserialize: failed to deserialize settings for %s\r\n", proc_type_label);
                                        break;
                                    }
                                }
                            }
                        }
                        else {
                            printf("deserialize: bad MIDI port number %u\r\n", midi_out_port);
                            result = false;
                            break;
                        }
                    }
                }
            }
            else {
                printf("deserialize: error got %u objects\r\n", nobjects);
                result = false;
            }
        }
        else {
            printf("deserialize: last preset %u not found\r\n", last_preset);
            result = false;
        }
    }
    if (root_value)
        json_value_free(root_value);
    if (result)
        dirty = false;
    return result;
}

bool rppicomidi::Midi_processor_manager::get_product_string_from_setting_data(char* json_format, char* product_string, size_t max_string)
{
    JSON_Value* root_value= json_parse_string(json_format);
    JSON_Object *root_object = NULL;
    bool result = false;
    if (root_value && json_value_get_type(root_value) == JSONObject) {
        root_object = json_value_get_object(root_value);
        if (json_object_has_value_of_type(root_object, "prod", JSONString)) {
            size_t len = json_object_get_string_len(root_object, "prod");
            if (len < max_string) {
                const char* prod = json_object_get_string(root_object, "prod");
                if (prod) {
                    strncpy(product_string, prod, max_string);
                    result = true;
                }
                else {
                    printf("error retrieving product string from JSON data\r\n");
                }
            }
        }
        else {
            printf("Could not parse product string from settings\r\n");
        }
    }
    if (root_value)
        json_value_free(root_value);
    return result;
}