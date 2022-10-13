/* MIT License
 *
 * Copyright (c) 2022 rppicomidi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/mutex.h"
#include "pico/bootrom.h"

#include "mono_graphics_lib.h"
#include "ssd1306i2c.h"
#include "ssd1306.h"
#include "view_manager.h"

#include "pio_usb.h"

#include "tusb.h"
#include "bsp/board.h"
#include "class/midi/midi_host.h"
#include "class/midi/midi_device.h"
#include "usb_descriptors.h"
#include "home_screen.h"
#include "nav_buttons.h"
#include "midi_processor.h"
#include "midi_processor_manager.h"

namespace rppicomidi {
class Pico_usb_midi_processor {
public:
    // Singleton Pattern

    /**
     * @brief Get the Instance object
     *
     * @return the singleton instance
     */
    static Pico_usb_midi_processor& instance()
    {
        static Pico_usb_midi_processor _instance; // Guaranteed to be destroyed.
                                             // Instantiated on first use.
        return _instance;
    }
    Pico_usb_midi_processor(Pico_usb_midi_processor const&) = delete;
    void operator=(Pico_usb_midi_processor const&) = delete;

    Pico_usb_midi_processor();
    void task();
    void clone_complete_cb();
    void poll_midi_dev_rx();
    void poll_midi_host_rx();

    const uint LED_GPIO=PICO_DEFAULT_LED_PIN;
    const uint8_t OLED_ADDR=0x3c;   // the OLED I2C address as a constant
    uint8_t addr[1];                // the OLED I2C address is stored here
    const uint8_t MUX_ADDR=0;       // no I2C mux
    uint8_t* mux_map=nullptr;       // no I2C mux
    const uint8_t OLED_SCL_GPIO = 19;   // The OLED SCL pin
    const uint8_t OLED_SDA_GPIO = 18;   // The OLED SDA pin

    // the i2c driver object
    Ssd1306i2c i2c_driver_oled{i2c1, addr, OLED_SDA_GPIO, OLED_SCL_GPIO, sizeof(addr), MUX_ADDR, mux_map};

    Ssd1306 ssd1306;    // the SSD1306 driver object
    Mono_graphics oled_screen; // the screen object
    View_manager oled_view_manager; // the view manager object
    uint8_t midi_dev_addr;  // The device address of the connected device
    enum {MIDI_DEVICE_NOT_INITIALIZED, MIDI_DEVICE_NEEDS_INIT, MIDI_DEVICE_IS_INITIALIZED} midi_device_status;

    static uint16_t render_done_mask;
    static void callback(uint8_t display_num)
    {
        render_done_mask |= (1u << display_num);
    }
    Home_screen home_screen;
    Nav_buttons nav_buttons;
};
}

uint16_t rppicomidi::Pico_usb_midi_processor::render_done_mask = 0;

rppicomidi::Pico_usb_midi_processor::Pico_usb_midi_processor()  : addr{OLED_ADDR},
    ssd1306{&i2c_driver_oled, 0, Ssd1306::Com_pin_cfg::ALT_DIS, 128, 64, 0, 0}, // set up the SSD1306 to drive at 128 x 64 oled
    oled_screen{&ssd1306, Display_rotation::Landscape180},                        // set up the screen for rotated landscape orientation
    midi_dev_addr{0},
    midi_device_status{MIDI_DEVICE_NOT_INITIALIZED},
    home_screen{oled_view_manager, oled_screen, "PICO MIDI PROCESSOR No Connected Device"},
    nav_buttons{oled_view_manager}
{
    gpio_init(LED_GPIO);
    gpio_set_dir(LED_GPIO, GPIO_OUT);

    // Set up the button GPIO
    gpio_init(BUTTON_UP);
    gpio_init(BUTTON_DOWN);
    gpio_init(BUTTON_LEFT);
    gpio_init(BUTTON_RIGHT);
    gpio_init(BUTTON_ENTER);
    gpio_init(BUTTON_BACK);
    gpio_init(BUTTON_SHIFT);
    gpio_set_dir(BUTTON_UP, GPIO_IN);
    gpio_set_dir(BUTTON_DOWN, GPIO_IN);
    gpio_set_dir(BUTTON_LEFT, GPIO_IN);
    gpio_set_dir(BUTTON_RIGHT, GPIO_IN);
    gpio_set_dir(BUTTON_ENTER, GPIO_IN);
    gpio_set_dir(BUTTON_BACK, GPIO_IN);
    gpio_set_dir(BUTTON_SHIFT, GPIO_IN);
    gpio_pull_up(BUTTON_UP);
    gpio_pull_up(BUTTON_DOWN);
    gpio_pull_up(BUTTON_LEFT);
    gpio_pull_up(BUTTON_RIGHT);
    gpio_pull_up(BUTTON_ENTER);
    gpio_pull_up(BUTTON_BACK);
    gpio_pull_up(BUTTON_SHIFT);

    render_done_mask = 0;
    int num_displays = 1;
    uint16_t target_done_mask = (1<<(num_displays)) -1;
    bool success = true;
    Midi_processor_manager::instance().set_screen(&oled_screen);
    oled_view_manager.push_view(&home_screen);
    oled_screen.render_non_blocking(callback, 0);
    while (success && render_done_mask != target_done_mask) {
        if (success) {
            success = oled_screen.task();
        }
    }
    assert(success);
}

void rppicomidi::Pico_usb_midi_processor::poll_midi_dev_rx()
{
    // device must be attached and have at least one endpoint ready to receive a message
    uint8_t packet[4];
    while (tud_midi_packet_read(packet)) {
        uint8_t cable = Midi_processor::get_cable_num(packet);
        if (Midi_processor_manager::instance().filter_midi_out(cable, packet)) {
            tuh_midi_packet_write(rppicomidi::Pico_usb_midi_processor::instance().midi_dev_addr, packet);
        }
    }
}

void rppicomidi::Pico_usb_midi_processor::poll_midi_host_rx(void)
{
    // device must be attached and have at least one endpoint ready to receive a message
    if (!midi_dev_addr  || !tuh_midi_configured(midi_dev_addr)) {
        return;
    }
    if (tuh_midih_get_num_rx_cables(midi_dev_addr) < 1) {
        return;
    }
    tuh_midi_read_poll(midi_dev_addr); // if there is data, then the callback will be called
}



void rppicomidi::Pico_usb_midi_processor::task()
{
    if (midi_device_status == MIDI_DEVICE_NEEDS_INIT) {
        uint8_t len = get_product_string_length();
        if (len > 0) {
            char prod[len+1];
            get_product_string(prod, len+1);
            uint8_t num_in_cables = tuh_midi_get_num_tx_cables(midi_dev_addr);
            uint8_t num_out_cables = tuh_midi_get_num_rx_cables(midi_dev_addr);
            home_screen.set_connected_device(prod, num_in_cables, num_out_cables);
            Midi_processor_manager::instance().set_connected_device(get_vid(), get_pid(), num_in_cables, num_out_cables);
            home_screen.draw();
        }
        tud_init(0);
        TU_LOG1("MIDI device initialized\r\n");
        midi_device_status = MIDI_DEVICE_IS_INITIALIZED;
    }
    else if (midi_device_status == MIDI_DEVICE_IS_INITIALIZED) {
        tud_task();
        if (tud_midi_mounted()) {
            poll_midi_dev_rx();
            Midi_processor_manager::instance().task();
        }
    }

    nav_buttons.poll();
    if (oled_screen.can_render()) {
        oled_screen.render_non_blocking(nullptr, 0);
    }
    oled_screen.task();

    // flash the Pico board LED
    static absolute_time_t previous_timestamp = {0};

    static bool led_state = false;

    absolute_time_t now = get_absolute_time();
    
    int64_t diff = absolute_time_diff_us(previous_timestamp, now);
    if (diff > 1000000) {
        gpio_put(rppicomidi::Pico_usb_midi_processor::instance().LED_GPIO, led_state);
        led_state = !led_state;
        previous_timestamp = now;
    }
}

//
// Below here are standard C code functions that glue the Pico SDK and
// tinyusb to the C++ code.
//

static void midi_host_app_task(void)
{
    if (cloning_is_required()) {
        if (rppicomidi::Pico_usb_midi_processor::instance().midi_dev_addr != 0) {
            TU_LOG1("start descriptor cloning\r\n");
            clone_descriptors(rppicomidi::Pico_usb_midi_processor::instance().midi_dev_addr );
        }
    }
    else if (clone_next_string_is_required()) {
        clone_next_string();
    }
    else if (descriptors_are_cloned()) {
        rppicomidi::Pico_usb_midi_processor::instance().poll_midi_host_rx();
        tuh_midi_stream_flush(rppicomidi::Pico_usb_midi_processor::instance().midi_dev_addr );
    }
}

// core1: handle host events
void core1_main()
{
    sleep_ms(10);
    multicore_lockout_victim_init(); // need to lockout core1 when core0 writes to flash
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

void device_clone_complete_cb()
{
    rppicomidi::Pico_usb_midi_processor::instance().clone_complete_cb();
}

void rppicomidi::Pico_usb_midi_processor::clone_complete_cb()
{
    printf("clone callback complete processor %u\r\n", get_core_num());
    midi_device_status = MIDI_DEVICE_NEEDS_INIT;
}

// core0: handle device events
int main()
{
    // default 125MHz is not appropreate. Sysclock should be multiple of 12MHz.
    set_sys_clock_khz(120000, true);

    sleep_ms(10);

    // direct printf to UART
    stdio_init_all();

    // all USB Host task run in core1
    multicore_reset_core1();    
    multicore_launch_core1(core1_main);

    // synchronize core1 startup
    while(!multicore_lockout_start_timeout_us(10000)) {
        // wait for lockout to work
    }
    while (!multicore_lockout_end_timeout_us(1000)) {
        // wait for unlock to work
    }
    // initialize the Pico_usb_midi_processor object instance
    auto instance_ptr=&rppicomidi::Pico_usb_midi_processor::instance();

    TU_LOG1("pico-usb-midi-processor\r\n");
    while (1) {
        instance_ptr->task();
    }

    return 0;
}

//--------------------------------------------------------------------+
// TinyUSB Callbacks
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. 
void tuh_midi_mount_cb(uint8_t dev_addr, uint8_t in_ep, uint8_t out_ep, uint8_t num_cables_rx, uint16_t num_cables_tx)
{
    TU_LOG1("MIDI device address = %u, IN endpoint %u has %u cables, OUT endpoint %u has %u cables\r\n",
        dev_addr, in_ep & 0xf, num_cables_rx, out_ep & 0xf, num_cables_tx);

    rppicomidi::Pico_usb_midi_processor::instance().midi_dev_addr = dev_addr;
    set_cloning_required();
}

// Invoked when device with hid interface is un-mounted
void tuh_midi_umount_cb(uint8_t dev_addr, uint8_t instance)
{
    rppicomidi::Pico_usb_midi_processor::instance().midi_dev_addr = 0;
    set_descriptors_uncloned();
    TU_LOG1("MIDI device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
}

void tuh_midi_rx_cb(uint8_t dev_addr, uint32_t num_packets)
{
    if (rppicomidi::Pico_usb_midi_processor::instance().midi_dev_addr == dev_addr) {
        while (num_packets>0) {
            --num_packets;
            uint8_t packet[4];
            if (tuh_midi_packet_read(dev_addr, packet)) {
                uint8_t cable = rppicomidi::Midi_processor::get_cable_num(packet);
                if (rppicomidi::Midi_processor_manager::instance().filter_midi_in(cable, packet)) {
                    tud_midi_packet_write(packet);
                }
            }
        }
    }
}

void tuh_midi_tx_cb(uint8_t dev_addr)
{
    (void)dev_addr;
}
