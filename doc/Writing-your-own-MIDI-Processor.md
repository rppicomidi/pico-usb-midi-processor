# Writing your own MIDI Processor
The MIDI Processors provided with the PUMP project are pretty flexible, but they cannot do everything.
You may have a special application that requires its own processor type. This document describes
how to write your own processor and how to plug it into the system so it will work just like
the provided processors.

# TL;DR
If you don't like to read documents and just want to get coding:
1. Find a midi_processor_* file set that is sort of similar to what you want and use it
as an example to implement your own processor. There are 4 files to create.
2. `#include` the new midi_processor_?????.h and midi_processor_?????_settings_view.h in the
midi_processor_manager.cpp file
3. Add the static_getname() and both static_make_new() methods to the Midi_processor_manager
class constructor.
4. Add the new midi_processor_?????.cpp and midi_processor_?????_settings_view.cpp files
to the CMakeLists.txt file.

# Overview
## Basic MIDI Processing Concepts
The system is designed to operate on 4-byte USB MIDI 1.0 packets. Each packet contains a 4-bit
virtual cable number plus 4-bits that describe the type of the packet and how many of the 3 subsequent
bytes contain valid data. When you write your own MIDI Processor, you are writing software that will,
for each packet received, either filter out the packet entirely or modify the packet into 1 or more new
packets. There are other optional features that are discussed later. There is a Midi_packet
class that helps you manage the USB MIDI packet field details.

Processing usually proceeds from USB MIDI IN to USB MIDI OUT for each virtual cable number. Virtual
cable numbers are determined by the USB device that you plugged to the USB Host connector of the PUMP.
Virtual cables are numbered from 0 and may be no greater than 15. A typical device will only have one
or two virtual cables, but some, especially multi-cable USB to MIDI converters, will have more. The
PUMP design refers to the signal processing on a virtual cable as a "chain." Most of the time, the
processing chain number corresponds to a virtual cable number.

Usually, a processor will sends output packets to the same chain on which it received them.
However, a processor could generate output packets that can route to any chain. Routing a packet to a new
chain is equivalent to receiving a new packet from a virtual MIDI cable on that chain, and all processing
on that chain will be applied to that packet. Note that each chain number has an MIDI IN path (from PUMP
host port to device port) and a MIDI OUT path (from PUMP device port to host port). The
PUMP system treats the two chains as separate processing paths.

## Settings and Presets
Most MIDI Processors need to be configured. During normal operation, settings are stored in normal C++
member variables. However, each class is responsible for serializing that data to JSON and deserializing
data from JSON. The library [parson](../ext_lib/parson/README.md) is responsible for handling the details
of the JSON syntax. Each MIDI Processor needs to have its own serialize_settings() and deserialize_settings()
implementation to match the data items that need to be saved and restored between each power cycle. The
Settings_file Object handles the actual interface with the flash memory.

## User Interface (UI)
Sometimes the hardest part of designing a MIDI processor is designing a usable UI that will work with the tiny
OLED screen and limited button controls. When you design your processor, try to sketch out the user inteface
before you write any code. The [pico-mono-ui-lib](../lib/pico-mono-ui-lib/README.md) describes how the UI
library works. The UI for each of the MIDI Processors PUMP provides can good starting points for design and
for when you get ready to code.

## More Advanced Processing Concepts

### Bi-directional (feedback) processing
Up until now, processors have been described as uni-directional. However, there are times when a processor
needs to operate in two directions. The channel button remap processor is a good example. Many MIDI controllers
have lighted buttons. When you press that button to toggle a DAW feature on and off, the MIDI controller sends
a channel message to the DAW, and then the DAW sends a related channel message back to control the button LED.
It is possible for the user to add a processor on the MIDI IN chain and then add a separate processor on the
corresponding MIDI OUT chain to accomplish this, but it is more convenient for the user to add a single processor
to the MIDI IN chain and have the corresponding feedback function to be automatically added to the corresponding
MIDI OUT chain. The Midi_processor_manager class does this automatically when you add a processor that has a
"feedback" function implemented.

### The processor task() function
Up until now, processors have been described to either filter out data or generate packets immediately. There
are applications where it is useful for a MIDI Processor to generate MIDI packets independently. For example,
a delay or echo processor may need to generate packets over time. An interpolation processor may need to generate
multiple packets to prevent a controlled parameter's value to from making steps so large that the sound generator
cannot track it. Etc. To handle this case, each MIDI processor may optionally have a task() function that the
Midi_processor_manager calls periodicaly. The task() function sends data on the assigned chain. There is
no bi-directional feedback task concept.

## The Midi_processor_manager Object
There is just one instance of the Midi_processor_manager class. That object is responsible for managing the
list of available processors, the processor list for each MIDI processing chain, and triggering save and
recall for presets, and for sequencing the process(), feedback() and task() methods for each processor.
It has data structures that tie the UI and settings for each MIDI Processor together.

# Details
## Implmenting a Subclass of Midi_processor
### Overview
Have a look at the file [midi_processor.h](../midi_processor.h). To create a new MIDI Processor, you must
create a subclass of the Midi_processor class and implement all of the functions you need
for your processor.
### File naming
I recommend that you name your files `midi_processor_????.cpp/h`, where `????` describes
what your processor does.

### Data In and Out, process(), task(), feedback() and fb_task()
The MIDI Processor `process()` function gets a packet to process by calling `proc_buf->pop()`.

The `process()` function may generate 0 outputs to filter out packets. It may generate
multiple packets for each input. It may also require multiple packets in before generating
any packets out. When processing is done, the `process()` function may store the processed
packet(s), if any, to any or all of three buffers:
- To pass the packet on to the next MIDI Processor in the chain, call `proc_buf->push()`
- To pass the packet to a start of new chain in the same direction as the current chain, call
`in_buf->push()`.
- To pass the packet to the start of a new chain in the direction opposite from the current 
chain, call `out_buf->push()`. Your MIDI Processor() does not need to override the
`process()` function. If it does not, the default `process()` function will just pass the
packet through unchanged.

The MIDI Processor `task()` function generates output packets. It can generate packets based
on previously received input packets that were passed to the `process()` or `feedback()`
functions, or it can generate them without external input (e.g., the clock generator
processor). The task() function should store any packets it generates any or all of the
same 3 buffers that the `process()` function uses. If your MIDI Processor overrides the
base class `task()` function, it must also override the `has_task_function()` to return true;

If a MIDI Processor overrides the `has_feedback_function()` method to return `true`, then
the MIDI Processor Manager will place the same MIDI Processor object it created for the
chain that calls the `process()` function the same-numberd chain in the opposite data flow
direction. For example, if the MIDI processor manager places a processor on MIDI IN chain 0,
(from the attached device to the host computer), then it will place the the same object,
flagged with as `is_feedback`, on the MIDI OUT chain 0 (from the host computer to the 
attached MIDI device). The MIDI processor manager will call the MIDI Processor's`feedback()`
function instead of the `process()` function.
- The `feedback()` function uses the `fb_buf` buffer instead of the `proc_buf` for in-chain packet handling.
- It uses the 'out_buf' to pass packets to the start of a new chain in the same direction as
the current chain.
- It uses the 'in_buf' to pass packets to the start of a new chain the the opposite direction
from the current chain.

The MIDI Processor `fb_task()` function generates output packets like the task() function
does. However, the `fb_task()` function generates packets to the same buffers as the feedback()
function. For example, a processor that turns on clock generation to the attached MIDI device
in response to a message from the attached MIDI device would implement the process() function
to handle the message to start clock generation and fb_task() to generate the clock output
messages.

### Settings implementation
The `lib/pico-mono-ui-lib` directory contains a number of settings classes. Values that your
MIDI Processor's UI class can modify should become part of the processor's settings. Each
setting should be declared in a MIDI Processor's private or protected data.

All the files that contain settings classes start with the prefix `setting_`. Each file
contains a description of the type of data that the class or template class can store. If
you need a setting type that is not there, feel free to create others. Bear in mind that the
UI for changing a setting value is limited to select, up arrow and down arrow. The value
also needs to able to be serialized from binary into JSON format and deserialized from
JSON format back to binary.

If your MIDI Processor has settings that the UI can change, your processor will have to
override the `load_defaults()`, `serialize_settings()` and `deserialize_settings()` methods.

## Implemented the MIDI Processor UI
The UI framework for the MIDI Processor mainly supports nested text menues. It is possible to
use the underlying graphics primatives to extend this framework to make something prettier.
Designing a usable UI for a tiny screen with few controls is difficult. I recommend making
as simple drawing with pencil and paper before you start coding. This may expose settings
that you didn't realize your MIDI Processor class needs.

### File naming
I recommend that you name your files `midi_processor_????_settings_view.cpp/h`, where the
file name up to the `????` is that same as the MIDI Processor class the UI view will control.

### User controls
Make sure you are familiar with the standard way the control buttons work. Read
[TUTORIAL](./TUTORIAL.md) and [Operating Instructions](../README.md#operating-instructions)
and design your UI to work in a way consistent with other modules.

## Integrating your MIDI Processor with the system
See the last steps of [TL;DR](#tldr).