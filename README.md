# pico-usb-midi-processor

The Pico USB MIDI Processor, or PUMP, is a Raspberry Pi Pico-based
general purpose device that processes USB MIDI data between a USB
Host such as a PC or Mac and a USB MIDI device such as a keyboard
or control surface. The PUMP has a simple OLED and button
graphical user interface, and it will retain setting between power
cycles.

The PUMP inspects and can modify or filter out every USB MIDI packet
between the MIDI device and USB host on every USB MIDI IN and MIDI OUT
port of the connected device. This allows you to create almost any
filter or processor for the MIDI data. For example

- Transpose a range of notes for a given MIDI channel
- Implement fader pickup to prevent large fader jumps on your DAW
- Remap control surface buttons to work better with your DAW
- Remap MIDI channels for a range of notes or controllers to implement keyboard splits.
- Filter out Real-time messages
- and so on

You can stack as many processors as you want to make a complex
processing chain for any MIDI port. The software framework is
flexible enough to allow new processing functions to be added.

# Hardware

The PUMP is based on a single Raspberry Pi Pico board. It uses the
native USB hardware to implment the USB MIDI device, and it uses
the Pico-PIO-USB project software plus a modified tinyusb stack
to implement the USB MIDI host. You set up the hardware to process
MIDI using a small SSD1306-based 128x64 dot monochrome OLED
display plus 7 buttons: 5 buttons for up, down, left, right, and
select plus a "Back/Home" button and a "Shift" button. I used 7
descrete buttons, but there are a number of ready-made assemblies
that use a 5-way "joystick" style switch for up/down/left/right/select
plus two more buttons to provide the 7-buttons. The OLED is a very
common I2C module that you can buy from any number of sources in
with white, blue or yellow dots.

The PUMP uses some of the Pico board's program memory flash chip
to store device settings, so the PUMP can remember the last
configuration for every device that was attached previously (until
it runs out of setting storage).

The PUMP uses 7 pins for the buttons, 2 pins for the USB Host, 2
pins for the OLED's I2C port, an 2 pins for a debug UART.

A photo of the hardware is below. 

# Software Build Instructions
## Set up your environment

The PUMP project uses original code plus a lot of code from other
projects on GitHub. Most are git submodules. All code is written
in C, C++, or the RP2040's PIO state machine assembly code. To
build it, you need to install the Pico C SDK version 1.4.0 or later.
Install this code plus the compiler toolchain per the instructions 
in the [Getting Started with Raspberry Pi Pico C/C++ development](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
document Chapter 2. Unless you are
using a Raspberry Pi for development, you will need to follow the
additional instructions in chapter 9 of the same document. I
recommend setting up Microsoft VS Code as described in chapter 7,
and I recommend installing OpenOCD and building a Picoprobe as
described in Appendix A for debugging and code development.

## Install and build the project source code
### Preview
This project uses the main application files plus some libries
from various GitHub projects.
- the Pico-PIO-USB project and a fork of the tinyusb project to
implement the USB communications; the fork was required to add
USB MIDI Host functionality
- the parson project to implement JSON format settings storage
- a fork of the pico-littlefs project to implement a file system
on the Pico program flash; the fork is required to handle writing
to flash whilst both RP2040 cores are active
- some modified font files from various projects for the OLEDs

Some original library code include
- pico-ssd1306-mono-graphics-lib, a higher performance C++ SSD1306
graphics library than others I tried. It also supports multiple
displays at once, which is useful for a future hardware attachment
I have planned for this project.
- pico-mono-ui-lib, a C++ UI library that supports multi-line menus
with scrolling, multi-screen navigation, the UI buttons, and
setting template classes that know how to serialize to JSON and deserialize
from JSON. It's not quite full MVC pattern, but it's not bad.

When you are installing the software, your directory structure
on your computer should look like this:
```
$PICO_DIR
    |
    +--pico-sdk
    |   |
    |   +--lib
    |       |
    |       +--tinyusb (from my forked code)
    |           |
    |          ...
    |           |
    |           +--hw
    |               |
    |               +--bsp
    |               |
    |               +--mcu
    |                   |
    |                  ...
    |                   |
    |                   +--raspberry_pi
    |                       |
    |                       +--Pico-PIO-USB (need to force
    |                           this submodule to install)
    |
    +--pico-usb-midi-processor
        |
        +--lib
        |   |
        |   +--pico-mono-graphics-lib
        |   |
        |   +--pico-mono-ui-lib
        |
        +--ext_lib
            |
            +--littlefs-lib (from my forked code)
            |
            +--parson
            |
            +--RPi-Pico-SSD1306-library
            |
            +--ssd1306
```

### Step By Step

To install on a Linux build host, use the following command line
commands. (A Mac Homebrew installation the same; I have no idea on a PC).
`$PICO_DIR` refers to the some top level directory where
you are are storing your Raspberry Pi Pico source code.

```
cd $PICO_DIR

# get the project source code and the library submodules
git clone https://github.com/rppicomidi/pico-usb-midi-processor.git
git submodule update --recursive --init

# make sure you have the latest Pico C SDK
cd pico-sdk
git pull
# get my fork of the tinyusb library with MIDI Host support
cd lib/tinyusb
git remote add upstream https://github.com/hathach/tinyusb.git
git remote set-url origin https://github.com/rppicomidi/tinyusb.git
git fetch origin
git checkout -b pio-midihost origin/pio-midihost
# get the Pico-PIO-USB submodule into the source tree
cd hw/mcu/raspberry_pi
git submodule update --init Pico-PIO-USB
```

To build on a Linux build host, use the following command line
commands. (A Mac Homebrew installation the same; I have no idea on a PC).
`$PICO_DIR` refers to the some top level directory where
you are are storing your Raspberry Pi Pico source code.
```
export PICO_SDK_PATH=$PICO_DIR/pico-sdk/
cd $PICO_DIR/pico-usb-midi-filter
mkdir build
cd build
cmake ..
make
```

# Operating Instructions

## Quick Start
Plug the PUMP's micro USB device port to a PC, Mac or other USB host.
The screen should show

```
PICO MIDI PROCESSOR
No Connected Device
```

Plug your keyboard or other MIDI device to the PUMP's USB Host Port.
You should see the device name on the top one or two lines of the
OLED followed by a list of MIDI IN and MIDI OUT ports that the
device normally exposes to your PC's or Mac's USB Host port.
`SETUP MIDI IN 1` should show in reverse video. MIDI IN and MIDI
OUT are from the USB Host's (the PC's or MAC's) perspective.

If you press the up or down buttons, you can navigate the list
of ports.  If there are more than 3 total MIDI IN or MIDI OUT ports,
then all won't fit on the screen. A vertical progress bar on the
right will appear to let you know you can navigate beyond what
is visible. The display will scroll as required. If you want
to scroll beyond what is visible in one go, press and hold the
Shift button before you press the Up or Down button.

If you press the select button, you can set up the
processing for the highlighted port.

In the Setup screen, you add a processor by pressing the Select
button whilst `Add new processor...` is highlighted. A list of
available processors will show up. Use the Up and Down buttons
to the processor you want to add and then press the Select button
to add it. The UI will return to the MIDI port setup screen. The
screen will show the processor added right above `Add new processor...`
If you changed your mind before you added the
processor, press the Back/Home button to return to the MIDI port
setup screen. To go all the way back to the home screen,
hold the Shift button and then press the Back/Home button.

If you added a processor by mistake, highlight the processor using
the Up and Down buttons and then press the left button to delete it.
There is no confirmation, so be careful. You can configure the processor
by using the Up or Down buttons to highlight it, then pressing select.

You adjust most processor parameters by using the Up and Down buttons
to choose the paramater, then press the Select button
to edit the paramater. In this edit mode, the Up and Down buttons
increment or decrement the paramter value. Holding the Up or Down
buttons will repeat the increment or decrement action. Press and
hold shift and the pressing Up or Down will increase the increment
or decrement interval. If there are multiple parameters on a line,
use a the Left button or Right button to choose the parameter to
increment or decrement.

## Processing paths

Data from the attached device goes to the what the PUMP calls "MIDI IN" ports.
Data from the attached USB Host (usually a PC or
Mac running a DAW) goes to what the PUMP calls "MIDI OUT" ports.
Most devices only have one MIDI IN and one MIDI OUT, but some have more.

The PUMP will process every MIDI packet
through every processor you add to a MIDI port before sending it
on to its destination. Processing is done in the same order you
added it in the GUI, so if you are doing something complex where
the order of processing matters, be sure to add the processors in the right order.

Some processors have "feedback" processing. For example, if you
remap a control surface button that has an LED, usually the DAW
will use the same message to control the LED that the control
surface sends to the DAW to indicate button press. For example,
if pressing the button sends NOTE ON 50, to the DAW, then the
DAW will send back NOTE ON 50 to light the button's LED. If
you assign a PUMP processor to remap control surface button's MIDI note
number to something else, then that processor needs a feedback
process to map the message from the DAW back to the message the
control surface expects. The `Channel Button Remap` processor does
exactly that. If you add a processor to PUMP MIDI IN port, then
the feedback process gets automatically added to the corresponding PUMP
MIDI OUT port. This "feedback" process does not show on the MIDI OUT
Setup on the OLED screen, but it is there.
