/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021, Ha Thach (tinyusb.org)
 *               2022 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/**
 * This program initializes pins GP1 and GP2 for use as a MIDI host port using
 * the Pico-PIO-USB library. It then waits for a USB device to be attached.
 * The software clones the USB descriptor of the attached USB device, and if
 * it is a MIDI device, it will initialized the RP2040 USB port as a MIDI
 * Device with the same descriptor as the attached device. It will transparently
 * pass data between the attached upstream USB host and the downstream USB MIDI
 * device. It will translate certain note messages from one note to another
 * note to allow DAW control buttons messages from an Arturia Keylab Essential 88
 * to work correctly with the Cubase DAW.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"

#include "pio_usb.h"

#include "tusb.h"
#include "bsp/board.h"
#include "class/midi/midi_host.h"
#include "class/midi/midi_device.h"
#include "usb_descriptors.h"
#include "midi_filter.h"
//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+
#define BUTTON_UP     11  // 6
#define BUTTON_DOWN   8   // 7
#define BUTTON_LEFT   10  // 8
#define BUTTON_RIGHT  6   // 9
#define BUTTON_ENTER  9   // 10
#define BUTTON_BACK   12  // 11
#define BUTTON_HOME   7   // 12
//--------------------------------------------------------------------+
// STATIC GLOBALS DECLARATION
//--------------------------------------------------------------------+
#ifdef PICO_DEFAULT_LED_PIN
static const uint LED_PIN = PICO_DEFAULT_LED_PIN;
#endif
static uint8_t midi_dev_addr = 0;
static const char* get_button_name(uint8_t button_map)
{
  for (int bit = 0; bit < 7; bit++) {
    uint8_t mask = 1 << bit;
    if (mask & button_map) {
      uint8_t button = bit + 6;
      switch (button) {
        case BUTTON_UP:
          return "UP";
        case BUTTON_DOWN:
          return "DOWN";
        case BUTTON_LEFT:
          return "LEFT";
        case BUTTON_RIGHT:
          return "RIGHT";
        case BUTTON_HOME:
          return "HOME";
        case BUTTON_BACK:
          return "BACK";
        case BUTTON_ENTER:
          return "ENTER";
        default:
          return "UNKNONW";
      }
    }
  }
}

static void poll_midi_dev_rx(bool connected)
{
  // device must be attached and have at least one endpoint ready to receive a message
  if (!connected)
  {
    return;
  }
  uint8_t packet[4];
  while (tud_midi_packet_read(packet))
  {
    if (filter_midi_out(packet))
      tuh_midi_packet_write(midi_dev_addr, packet);
  }
}

static void poll_midi_host_rx(void)
{
  // device must be attached and have at least one endpoint ready to receive a message
  if (!midi_dev_addr || !tuh_midi_configured(midi_dev_addr))
  {
    return;
  }
  if (tuh_midih_get_num_rx_cables(midi_dev_addr) < 1)
  {
    return;
  }
  tuh_midi_read_poll(midi_dev_addr); // if there is data, then the callback will be called
}

static void midi_host_app_task(void)
{
  if (cloning_is_required()) {
    if (midi_dev_addr != 0) {
      TU_LOG1("start descriptor cloning\r\n");
      clone_descriptors(midi_dev_addr);
    }
  }
  else if (clone_next_string_is_required()) {
    clone_next_string();
  }
  else if (descriptors_are_cloned()) {
    poll_midi_host_rx();
    tuh_midi_stream_flush(midi_dev_addr);
  }
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep, uint8_t num_cables_rx, uint16_t num_cables_tx)
{
  TU_LOG1("MIDI device address = %u, IN endpoint %u has %u cables, OUT endpoint %u has %u cables\r\n",
      dev_addr, in_ep & 0xf, num_cables_rx, out_ep & 0xf, num_cables_tx);

  midi_dev_addr = dev_addr;
  set_cloning_required();
}

// Invoked when device with hid interface is un-mounted
void tuh_midi_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  midi_dev_addr = 0;
  set_descriptors_uncloned();
  TU_LOG1("MIDI device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
}

void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets)
{
  if (midi_dev_addr == dev_addr)
  {
    while (num_packets>0)
    {
      --num_packets;
      uint8_t packet[4];
      if (tuh_midi_packet_read(dev_addr, packet))
      {
        if (filter_midi_in(packet))
          tud_midi_packet_write(packet);
      }
    }
  }
}

void tuh_midi_tx_cb(uint8_t dev_addr)
{
    (void)dev_addr;
}

static void led_blinking_task(void);
static void poll_buttons_task(void);

// core1: handle host events
void core1_main() {
  sleep_ms(10);

  // Use tuh_configure() to pass pio configuration to the host stack
  // Note: tuh_configure() must be called before
  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);

  // To run USB SOF interrupt in core1, init host stack for pio_usb (roothub
  // port1) on core1
  tuh_init(1);

  while (true) {
    tuh_task(); // tinyusb host task

    midi_host_app_task();
  }
}
static enum {MIDI_DEVICE_NOT_INITIALIZED, MIDI_DEVICE_NEEDS_INIT, MIDI_DEVICE_IS_INITIALIZED} midi_device_status = MIDI_DEVICE_NOT_INITIALIZED;
void device_clone_complete_cb()
{
  midi_device_status = MIDI_DEVICE_NEEDS_INIT;
}

// core0: handle device events
int main(void) {
  // default 125MHz is not appropreate. Sysclock should be multiple of 12MHz.
  set_sys_clock_khz(120000, true);

  sleep_ms(10);

  // direct printf to UART
  stdio_init_all();

  multicore_reset_core1();
  // all USB task run in core1
  multicore_launch_core1(core1_main);

#ifdef PICO_DEFAULT_LED_PIN
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
#endif

  // Set up the button GPIO
  gpio_init(BUTTON_UP);
  gpio_init(BUTTON_DOWN);
  gpio_init(BUTTON_LEFT);
  gpio_init(BUTTON_RIGHT);
  gpio_init(BUTTON_ENTER);
  gpio_init(BUTTON_BACK);
  gpio_init(BUTTON_HOME);
  gpio_set_dir(BUTTON_UP, GPIO_IN);
  gpio_set_dir(BUTTON_DOWN, GPIO_IN);
  gpio_set_dir(BUTTON_LEFT, GPIO_IN);
  gpio_set_dir(BUTTON_RIGHT, GPIO_IN);
  gpio_set_dir(BUTTON_ENTER, GPIO_IN);
  gpio_set_dir(BUTTON_BACK, GPIO_IN);
  gpio_set_dir(BUTTON_HOME, GPIO_IN);
  gpio_pull_up(BUTTON_UP);
  gpio_pull_up(BUTTON_DOWN);
  gpio_pull_up(BUTTON_LEFT);
  gpio_pull_up(BUTTON_RIGHT);
  gpio_pull_up(BUTTON_ENTER);
  gpio_pull_up(BUTTON_BACK);
  gpio_pull_up(BUTTON_HOME);
  TU_LOG1("pico-usb-midi-filter\r\n");
  filter_midi_init();
  while (1)
  {
    if (midi_device_status == MIDI_DEVICE_NEEDS_INIT) {
      tud_init(0);
      TU_LOG1("MIDI device initialized\r\n");
      midi_device_status = MIDI_DEVICE_IS_INITIALIZED;
    }
    else if (midi_device_status == MIDI_DEVICE_IS_INITIALIZED) {
      tud_task();
      bool connected = tud_midi_mounted();
      poll_midi_dev_rx(connected);
    }
    
    led_blinking_task();
    poll_buttons_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

//--------------------------------------------------------------------+
// Blinking Task
//--------------------------------------------------------------------+

static void led_blinking_task(void)
{
#ifndef PICO_DEFAULT_LED_PIN
#warning led_blinking_task requires a board with a regular LED
#else

  const uint32_t interval_ms = 1000;
  static uint32_t start_ms = 0;

  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  gpio_put(LED_PIN,led_state);
  led_state = 1 - led_state; // toggle
#endif
}

static void poll_buttons_task(void)
{
  static uint8_t prev_buttons = 0;
  uint8_t buttons = (uint8_t)(~(gpio_get_all() >> 6) & 0x7f);
  if (buttons != prev_buttons) {
    printf("buttons=%02x\r\n",buttons);
    if (buttons) {
      printf("button=%s\r\n", get_button_name(buttons));
    }
    prev_buttons = buttons;
  }
}