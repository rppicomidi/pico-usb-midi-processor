#include "midi_processor_factory.h"
#include "midi_processor_mc_fader_pickup.h"
#include "midi_processor_transpose.h"
uint16_t rppicomidi::Midi_processor_factory::unique_id = 0;
rppicomidi::Midi_processor_factory::Midi_processor_factory()
{
    // Note: try to add new processor types to this list alphabetically
    mutex_init(&processing_mutex);
    proclist.push_back({Midi_processor_mc_fader_pickup::static_getname(), Midi_processor_mc_fader_pickup::static_make_new});
    proclist.push_back({Midi_processor_transpose::static_getname(), Midi_processor_transpose::static_make_new});
}

void rppicomidi::Midi_processor_factory::set_connected_device(uint16_t vid_, uint16_t pid_, uint8_t num_in_cables_, uint8_t num_out_cables_)
{
    // TODO: use vid_ and pid_ to generate settings file name and recall settings on device connect if settings exist
    (void)vid_;
    (void)pid_;
    for (int cable = 0; cable < num_in_cables_; cable++) {
        midi_in_processors.push_back(std::vector<Midi_processor*>());
        midi_in_proc_fns.push_back(std::vector<Midi_processor_fn>());
    }
    for (int cable = 0; cable < num_out_cables_; cable++) {
        midi_out_processors.push_back(std::vector<Midi_processor*>());
        midi_out_proc_fns.push_back(std::vector<Midi_processor_fn>());
    }
}

void rppicomidi::Midi_processor_factory::add_new_midi_processor_by_idx(size_t idx, uint8_t cable, bool is_midi_in)
{
    if (idx < proclist.size()) {
        auto proc = proclist[idx].processor(unique_id++);
        mutex_enter_blocking(&processing_mutex);
        if (is_midi_in) {
            midi_in_processors[cable].push_back(proc);
        }
        else {
            midi_out_processors[cable].push_back(proc);
        }
        build_processor_structures();
        mutex_exit(&processing_mutex);
    }
}

void rppicomidi::Midi_processor_factory::build_processor_structures()
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
            midi_in_proc_fns[cable].push_back(Midi_processor_fn{midi_in_proc, false});
            if (midi_in_proc->has_feedback_process() && cable < midi_out_proc_fns.size()) {
                midi_out_proc_fns[cable].push_back(Midi_processor_fn{midi_in_proc, true});
            }
            if (midi_in_proc->has_task()) {
                processors_with_tasks.push_back(midi_in_proc);
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
            midi_out_proc_fns[cable].push_back(Midi_processor_fn{midi_out_proc, false});
            if (midi_out_proc->has_feedback_process() && cable < midi_in_proc_fns.size()) {
                midi_in_proc_fns[cable].push_back(Midi_processor_fn{midi_out_proc, true});
            }
            if (midi_out_proc->has_task()) {
                processors_with_tasks.push_back(midi_out_proc);
            }
        }
    }
}

bool rppicomidi::Midi_processor_factory::filter_midi_in(uint8_t cable, uint8_t* packet)
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


bool rppicomidi::Midi_processor_factory::filter_midi_out(uint8_t cable, uint8_t* packet)
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

void rppicomidi::Midi_processor_factory::task()
{
    mutex_enter_blocking(&processing_mutex);
    for (auto& proc: processors_with_tasks) {
        proc->task();
    }
    mutex_exit(&processing_mutex);
}
