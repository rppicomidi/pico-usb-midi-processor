/**
 * @file backup_view.cpp
 *
 * MIT License
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
#include "backup_view.h"
#include "settings_file.h"

rppicomidi::Backup_view::Backup_view(Mono_graphics& screen_) : 
    View{screen_, screen_.get_clip_rect()}, font{screen.get_font_12()}, menu{screen, static_cast<uint8_t>(font.height*2), font}
{
    backup_all = new Callback_menu_item("Backup All Presets",screen, font, this, static_start_backup, Select_result::exit_view);
    assert(backup_all);
    menu.add_menu_item(backup_all);
}

void rppicomidi::Backup_view::entry()
{
    menu.entry();
}

void rppicomidi::Backup_view::draw()
{
    screen.clear_canvas();
    screen.center_string(font, "Next Backup Folder", 0);
    char dirname[20];
    bool result = Settings_file::instance().get_next_backup_directory_name(dirname, sizeof(dirname));
    if (result) {
        screen.center_string(font, "Next Backup Folder", 0);
        screen.center_string(font, dirname, font.height);
        menu.draw();
    }
    else {
        screen.center_string(font, "Backup Folder Failed", 0);
        return;
    }
}

void rppicomidi::Backup_view::static_start_backup(View* view_, View**)
{
    FRESULT res = Settings_file::instance().backup_all_presets();
    if (res != FR_OK) {
        printf("error %u backing up presets\r\n", res);
        auto me = reinterpret_cast<Backup_view*>(view_);
        me->backup_all->set_select_action(Select_result::no_op);
        me->screen.center_string(me->font, "Backup Failed",0);
    }
}
