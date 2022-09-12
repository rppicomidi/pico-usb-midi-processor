#pragma once
#include <cstdint>
#include <string>
namespace rppicomidi {
class Midi_processor_element
{
public:
    Midi_processor_element(const char* name_) : name{name_} {restore_defaults();}
    const char* get_name();

    /**
     * @brief restore any setting for the processor element to default
     */
    virtual void restore_defaults() {}

    /**
     * @brief Process the input MIDI stream and generate an output MIDI stream up
     * to max_midi_stream_out bytes long.
     * 
     * @param midi_stream_in a pointer to the MIDI stream to be processed
     * @param len_midi_stream_in the number of bytes in the MIDI stream
     * @param cable_in is the virtual MIDI cable number 0-15
     * @param midi_stream_out a pointer to the output MIDI stream bytes
     * @param max_midi_stream_out the maximum number of bytes that the
     * midi_stream_out buffer can hold
     * @param cable_out is a pointer to the location where the stream
     * output cable number is stored.
     * @return uint32_t the number of bytes in midi_stream_out. If the
     * process filters out certain MIDI packets or needs more of the MIDI
     * stream to generate an appropriate output, then midi_stream_out 
     * may contain 0 bytes. so the return value may be 0.
     */
    virtual uint32_t process(uint8_t* midi_stream_in, uint32_t len_midi_stream_in, uint8_t cable_in, 
        uint8_t* midi_stream_out, uint32_t max_midi_stream_out, uint8_t* cable_out) = 0;

    /**
     * @brief Generate a JSON string that represents the processor block name and settings
     * 
     * @return std::string 
     */
    virtual std::string serialize() {}

    /**
     * @brief deserialize the JSON settings string
     * 
     * @param settings the JSON settings string
     * @return true if the setting string deserialized successfully
     */
    virtual bool deserialize(std::string& settings) {return false; }
private:
    const char* name;
};
}