cmake_minimum_required(VERSION 3.13)

add_library(rp2040_rtc INTERFACE)
target_sources(rp2040_rtc INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_rtc.cpp
)
target_include_directories(rp2040_rtc INTERFACE
 ${CMAKE_CURRENT_LIST_DIR}
)
target_link_libraries(rp2040_rtc INTERFACE pico_stdlib hardware_rtc)

