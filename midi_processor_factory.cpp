#include "midi_processor_factory.h"
#include "midi_processor_mc_fader_pickup.h"
#include "midi_processor_transpose.h"
uint16_t rppicomidi::Midi_processor_factory::unique_id = 0;
rppicomidi::Midi_processor_factory::Midi_processor_factory(std::vector<std::vector<Midi_processor*>>& midi_in_processors_,
    std::vector<std::vector<Midi_processor*>>& midi_out_processors_,
    std::vector<std::vector<Midi_processor_fn>>& midi_in_proc_fns_,
    std::vector<std::vector<Midi_processor_fn>>& midi_out_proc_fns_,
    std::vector<Midi_processor*>& processors_with_tasks_) : midi_in_processors{midi_in_processors_},
        midi_out_processors{midi_out_processors_}, midi_in_proc_fns{midi_in_proc_fns_},
        midi_out_proc_fns{midi_out_proc_fns_}, processors_with_tasks{processors_with_tasks_}
{
    proclist.push_back({Midi_processor_mc_fader_pickup::static_getname(), Midi_processor_mc_fader_pickup::make_new});
    proclist.push_back({Midi_processor_transpose::static_getname(), Midi_processor_transpose::make_new});
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