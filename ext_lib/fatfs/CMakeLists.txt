# This CMakeLists.txt file is not part of "FatFs Module Source Files R0.14b"
# It is provided to make integration with the rest of the project simpler
#
# MIT License
#
# Copyright (c) 2022 rppicomidi
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.13)

add_library(fatfs INTERFACE)
set(FATFSPATH, ${CMAKE_CURRENT_LIST_DIR}/source)
target_sources(fatfs INTERFACE
    ${FATFSPATH}/ff.c
    ${FATFSPATH}/ffsystem.c
    ${FATFSPATH}/ffunicode.c
    ${FATFSPATH}/diskio.c
)
target_include_directories(fatfs INTERFACE ${CMAKE_CURRENT_LIST_DIR} ${PICO_TINYUSB_PATH})
target_link_libraries(fatfs INTERFACE pico_stdlib tinyusb_host)
