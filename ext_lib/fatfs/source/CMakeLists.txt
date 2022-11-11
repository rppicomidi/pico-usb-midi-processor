cmake_minimum_required(VERSION 3.13)

add_library(msc_fatfs INTERFACE)
target_sources(msc_fatfs INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ff.c
    ${CMAKE_CURRENT_LIST_DIR}/ffsystem.c
    ${CMAKE_CURRENT_LIST_DIR}/ffunicode.c
    ${CMAKE_CURRENT_LIST_DIR}/diskio.c
)
target_include_directories(msc_fatfs INTERFACE ${CMAKE_CURRENT_LIST_DIR})


