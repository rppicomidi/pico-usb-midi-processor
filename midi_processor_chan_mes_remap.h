#pragma once
#include <map>
#include "midi_processor.h"
#include "setting_number.h"
#include "setting_string_enum.h"
#include "setting_bimap.h"
#include "pico/mutex.h"
namespace rppicomidi
{
    class Midi_processor_chan_mes_remap : public Midi_processor
    {
    public:
        Midi_processor_chan_mes_remap(uint16_t unique_id);
        Midi_processor_chan_mes_remap() = delete;
        virtual ~Midi_processor_chan_mes_remap()=default;
        size_t add_remap()
        {
            mutex_enter_blocking(&processing_mutex);
            auto result = bimap.push_back(bimap.get_max(), bimap.get_max());
            mutex_exit(&processing_mutex);
            return result;
        }
        void delete_remap(size_t idx)
        {
            mutex_enter_blocking(&processing_mutex);
            bimap.erase(idx);
            mutex_exit(&processing_mutex);
        }
        bool process(uint8_t *packet) final { return process_internal(packet, 0, 1); }
        const std::vector<std::string>* get_all_possible_channel_message_types() const { return message_type.get_all_possible_values(); }
        bool set_message_type(size_t idx) { dirty = message_type.get_ivalue() != (int)idx; return message_type.set(idx); }
        void get_message_type(std::string &typestr) { message_type.get(typestr); }
        bool set_display_format(size_t idx) { dirty = true; return display_format.set(idx); }
        void get_display_format(std::string &typestr) { display_format.get(typestr); }
        size_t get_display_format() {return display_format.get_ivalue(); }
        const std::vector<std::string>* get_all_possible_display_formats() const { return display_format.get_all_possible_values(); }
        void serialize_settings(const char* name, JSON_Object *root_object) final;
        bool deserialize_settings(JSON_Object *root_object) final;
        static uint8_t static_get(void *context_, size_t bimap_idx_, size_t element_idx_)
        {
            auto me = reinterpret_cast<Midi_processor_chan_mes_remap *>(context_);
            return me->bimap.get(bimap_idx_, element_idx_);
        }
        static uint8_t static_incr(void *context_, size_t bimap_idx_, size_t element_idx_, int delta_)
        {
            auto me = reinterpret_cast<Midi_processor_chan_mes_remap *>(context_);
            auto current = me->bimap.get(bimap_idx_, element_idx_);
            auto newval = me->bimap.incr(bimap_idx_, element_idx_, delta_);
            me->dirty = current != newval;

            return newval;
        }
        static uint8_t static_chan_get(void* context_)
        {
            auto me = reinterpret_cast<Midi_processor_chan_mes_remap *>(context_);
            return me->chan.get();
        }
        static uint8_t static_chan_incr(void* context_, int delta)
        {
            auto me = reinterpret_cast<Midi_processor_chan_mes_remap *>(context_);
            auto current = me->chan.get();
            auto newval = me->chan.incr(delta);
            me->dirty = current != newval;

            return newval;
        }
        static const char *static_getname() { return "Channel Message Remap"; }
        static Midi_processor* static_make_new(uint16_t unique_id_) { return new Midi_processor_chan_mes_remap(unique_id_); }
    protected:
        bool process_internal(uint8_t *packet, size_t first_idx, size_t second_idx);
        const std::string note_msg;
        const std::string cc_msg;
        const std::string poly_pressure_msg;
        const std::string chan_pressure_msg;
        const std::string prog_change_msg;
        const std::string format_decimal;
        const std::string format_hex;
        Setting_number<uint8_t> chan;
        Setting_string_enum message_type;
        Setting_bimap<uint8_t> bimap;
        Setting_string_enum display_format;
        mutex processing_mutex;
    };
}