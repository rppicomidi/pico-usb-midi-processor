#include <cstdio>
#include "setup_menu.h"

rppicomidi::Setup_menu* rppicomidi::Setup_menu::_instance = nullptr;

rppicomidi::Setup_menu::Setup_menu(Mono_graphics& screen_, const Rectangle& rect_, Midi_processor_model& model_, Settings_file& settings_) : 
    Menu(screen_, rect_), model{model_}, save_exit{screen_,0,0, screen_.get_font_12(),std::string("Save & Exit"), settings_},
    quit_exit{screen_, 0, screen_.get_font_12().height, screen_.get_font_12(), std::string("Quit without saving")} /*,
    mc_port{screen_, Rectangle(0, screen_.get_font_12().height*2, screen_.get_screen_width(), screen_.get_font_12().height),
        std::string("MC MIDI Port:"), screen.get_font_12(), get_mc_port_value, set_mc_port_value, 1, 16,  false} */
{
    assert(_instance == nullptr);
    _instance = this;
    add_menu_item(&save_exit);
    add_menu_item(&quit_exit);
    //add_menu_item(&mc_port);
}

void rppicomidi::Setup_menu::entry()
{
    uint8_t cables_in, cables_out;
    model.get_num_cables(cables_in, cables_out);
    if (cables_in != cables_out)
        printf("Warning: cables_in=%u cables_out=%u\r\n", cables_in, cables_out);
    //mc_port.set_limits(1,cables_in);
    #if 0
    if (cables_in == 1)
        mc_port.set_disabled(true);
    else
        mc_port.set_disabled(false);
    #endif
    Menu::entry();
    for (auto& item: items) {
        item->set_selected(item == *current_item);
    }
}

#if 0
int rppicomidi::Setup_menu::get_mc_port_value()
{
    uint8_t cable_in, cable_out;
    _instance->model.get_mc_cable(cable_in, cable_out);
    if (cable_in != cable_out)
        printf("warning: cable_in=%u cable_out=%u\r\n", cable_in, cable_out);
    return cable_in+1;
}

void rppicomidi::Setup_menu::set_mc_port_value(int value_)
{
    uint8_t cable = (uint8_t)(value_-1);
    _instance->model.set_mc_cable(cable, cable);
}

#endif