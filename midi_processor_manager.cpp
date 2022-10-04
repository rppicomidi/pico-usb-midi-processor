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
    // TODO: use vid_ and pid_ to generate settings file name and recall settings on device connect if settings exist
    (void)vid_;
    (void)pid_;
    for (int cable = 0; cable < num_in_cables_; cable++) {
        midi_in_processors.push_back(std::vector<Mpv_element>());
        midi_in_proc_fns.push_back(std::vector<Midi_processor_fn>());
    }
    for (int cable = 0; cable < num_out_cables_; cable++) {
        midi_out_processors.push_back(std::vector<Mpv_element>());
        midi_out_proc_fns.push_back(std::vector<Midi_processor_fn>());
    }
}

rppicomidi::Midi_processor_settings_view*  rppicomidi::Midi_processor_manager::add_new_midi_processor_by_idx(size_t idx, uint8_t cable, bool is_midi_in)
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
