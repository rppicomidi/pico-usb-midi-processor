cmake_minimum_required(VERSION 3.13)
include (pico_sdk_import.cmake)
project(pico_usb_midi_processor)

pico_sdk_init()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPICO_USE_MALLOC_MUTEX=1")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ext_lib/littlefs-lib ext_lib/littlefs-lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/pico-ssd1306-mono-graphics-lib lib/pico-ssd1306-mono-graphics-lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/pico-mono-ui-lib lib/pico-mono-ui-lib)

set(target_name midi_app)
add_executable(${target_name})

pico_enable_stdio_uart(${target_name} 1) 

set(PICO_PIO_USB_SRC "${PICO_TINYUSB_PATH}/hw/mcu/raspberry_pi/Pico-PIO-USB/src")

pico_generate_pio_header(${target_name} ${PICO_PIO_USB_SRC}/usb_tx.pio)
pico_generate_pio_header(${target_name} ${PICO_PIO_USB_SRC}/usb_rx.pio)

target_sources(${target_name} PRIVATE
# midi_app.c
 pico-usb-midi-processor.cpp
 usb_descriptors.c
 keylab_essential_mc_filter.c
 midi_mc_fader_pickup.c
 midi_processor_model.cpp
 settings_file.cpp
 #setup_menu.cpp
 home_screen.cpp
 ${PICO_PIO_USB_SRC}/pio_usb.c
 ${PICO_PIO_USB_SRC}/pio_usb_device.c
 ${PICO_PIO_USB_SRC}/pio_usb_host.c
 ${PICO_PIO_USB_SRC}/usb_crc.c

 ${CMAKE_CURRENT_LIST_DIR}/ext_lib/parson/parson.c

 ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/dcd_pio_usb.c
 ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
 )
target_link_options(${target_name} PRIVATE -Xlinker --print-memory-usage)
target_compile_options(${target_name} PRIVATE -Wall -Wextra)
target_compile_definitions(${target_name} PRIVATE
PICO_DEFAULT_UART_TX_PIN=16
PICO_DEFAULT_UART_RX_PIN=17
)
target_include_directories(${target_name} PRIVATE ${PICO_PIO_USB_SRC} ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/ext_lib/parson)

target_link_libraries(${target_name} PRIVATE pico_stdlib pico_multicore hardware_pio hardware_dma tinyusb_device tinyusb_host 
ssd1306 ssd1306i2c text_box mono_graphics_lib ui_menu ui_view_manager ui_nav_buttons littlefs-lib)
pico_add_extra_outputs(${target_name})

