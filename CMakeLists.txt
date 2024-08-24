cmake_minimum_required(VERSION 3.13)
include (pico_sdk_import.cmake)
project(pico_usb_midi_processor)
#override the TinyUSB debug level set in family.cmake
#set(LOG 3)
if (NOT DEFINED PICO_BOARD)
set (PICO_BOARD pico)
endif()
pico_sdk_init()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ext_lib/littlefs-lib ext_lib/littlefs-lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/pico-ssd1306-mono-graphics-lib lib/pico-ssd1306-mono-graphics-lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/pico-mono-ui-lib lib/pico-mono-ui-lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/usb_midi_host)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/usb_midi_dev_ac_optional)

set(EMBEDDED_CLI_PATH ${CMAKE_CURRENT_LIST_DIR}/ext_lib/embedded-cli/lib/)
add_executable(midi_processor)

pico_enable_stdio_uart(midi_processor 1) 

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ext_lib/fatfs/source)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/rp2040_rtc)
target_sources(midi_processor PRIVATE
 pico-usb-midi-processor.cpp
 usb_descriptors.c
 midi_processor_mc_fader_pickup.cpp
 midi_processor_transpose.cpp
 settings_file.cpp
 home_screen.cpp
 midi_processor_manager.cpp
 midi_processor_setup_screen.cpp
 midi_processor_mc_fader_pickup_settings_view.cpp
 midi_processor_transpose_view.cpp
 midi_processor_chan_mes_remap.cpp
 midi_processor_chan_mes_remap_settings_view.cpp
 preset_view.cpp
 clock_set_view.cpp
 backup_view.cpp
 restore_view.cpp
 settings_flash_view.cpp

 ${CMAKE_CURRENT_LIST_DIR}/ext_lib/parson/parson.c

 ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/dcd_pio_usb.c
 ${PICO_TINYUSB_PATH}/src/portable/raspberrypi/pio_usb/hcd_pio_usb.c
 ${EMBEDDED_CLI_PATH}/src/embedded_cli.c
)
target_link_options(midi_processor PRIVATE -Xlinker --print-memory-usage)
target_compile_options(midi_processor PRIVATE -Wall -Wextra)
target_compile_definitions(midi_processor PRIVATE
PICO_HEAP_SIZE=0x20000
PICO_USE_MALLOC_MUTEX=1
)
if (PICO_BOARD MATCHES pico)
target_compile_definitions(midi_processor PRIVATE
PICO_DEFAULT_UART_TX_PIN=16
PICO_DEFAULT_UART_RX_PIN=17
PICO_DEFAULT_PIO_USB_DP_PIN=0
)
elseif(PICO_BOARD MATCHES adafruit_feather_rp2040_usb_host)
target_compile_definitions(midi_processor PRIVATE
PICO_DEFAULT_UART_TX_PIN=0
PICO_DEFAULT_UART_RX_PIN=1
PICO_DEFAULT_PIO_USB_DP_PIN=16
OLED_SDA_GPIO=2
OLED_SCL_GPIO=3
BUTTON_UP=13
BUTTON_DOWN=12
BUTTON_LEFT=11
BUTTON_RIGHT=10
BUTTON_ENTER=9
BUTTON_BACK=6
BUTTON_SHIFT=5
USE_ADAFRUIT_FEATHER_RP2040_USBHOST=1
)
endif()

target_include_directories(midi_processor PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/ext_lib/parson
  ${EMBEDDED_CLI_PATH}/include
  ${FATFS_PATH}
)

target_link_libraries(midi_processor PRIVATE pico_stdlib pico_multicore hardware_pio hardware_dma
tinyusb_board tinyusb_device tinyusb_host tinyusb_pico_pio_usb usb_midi_host_app_driver usb_midi_device_app_driver
ssd1306 ssd1306i2c text_box mono_graphics_lib ui_menu ui_view_manager ui_nav_buttons ui_text_item_chooser littlefs-lib rp2040_rtc msc_fatfs)
pico_add_extra_outputs(midi_processor)

