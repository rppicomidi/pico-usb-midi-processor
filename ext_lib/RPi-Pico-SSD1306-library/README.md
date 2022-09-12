# This README.md file and the file font.hpp in this directory come from https://github.com/mbober1/RPi-Pico-SSD1306-library.git
# License info for this directory should be available there.

# Raspberry Pico SSD1306 + GFX Library

Based on Adafruit GFX Library
https://github.com/adafruit/Adafruit-GFX-Library


# Usage

## Hardware
Connect your SSD1306 oled display to i2c1 port (you can change it during initialization)
```
SDA->GPIO2
SCL->GPIO3
GND->GND
VCC->3V3(OUT)
```

## Software
The library needs only two files to run `SSD1306.hpp` and `SSD1306.cpp`.
This gives you the ability to display a bitmap or array of pixels. 
You need the GFX library to make it easier to create images for your display. (`GFX.hpp` and `GFX.cpp`)

`logo.hpp` is an example showing how to create bitmaps.
`fonts.hpp` contains one font.

## Example
Repo provides an example of how a library can be used.

```
git clone https://github.com/mbober1/RPi-Pico-SSD1306-library.git
cd RPi-Pico-SSD1306-library/example/
mkdir build
cd build
```
Change path to pico-sdk in CMakeLists.txt (line 3)
```
cmake ..
make -j4
```

## Make your own logo
http://www.majer.ch/lcd/adf_bitmap.php

## Features
- [X] Support 128x32 displays
- [ ] DMA Support
- [ ] Description of each function
