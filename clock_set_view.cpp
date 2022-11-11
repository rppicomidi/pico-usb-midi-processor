#include <cstdio>
#include "clock_set_view.h"

void rppicomidi::Clock_set_view::entry()
{
    Rp2040_rtc::instance().get_date(year, month, day);
    Rp2040_rtc::instance().get_time(hour, minute, second);
}

void rppicomidi::Clock_set_view::draw_field(uint8_t field_idx_, bool selected_)
{
    char field[5];
    Pixel_state fg = selected_ ? Pixel_state::PIXEL_ZERO:Pixel_state::PIXEL_ONE;
    Pixel_state bg = selected_ ? Pixel_state::PIXEL_ONE:Pixel_state::PIXEL_ZERO;
    uint8_t x = x_offset;
    uint8_t y = font.height * 3;
    uint8_t field_width = 2;
    switch(field_idx_) {
        default:
            return;
            break;
        case month_idx:
            sprintf(field, "%02u", month);
            break;
        case day_idx:
            sprintf(field, "%02u", day);
            x += font.width*3;
            break;
        case year_idx:
            if (year > 9999)
                return;
            sprintf(field, "%04u", year);
            x += font.width*6;
            field_width = 4;
            break;
        case hour_idx:
            sprintf(field, "%02u", hour);
            x += font.width*11;
            break;
        case min_idx:
            sprintf(field, "%02u", minute);
            x += font.width*14;
            break;
        case sec_idx:
            sprintf(field, "%02u", second);
            x += font.width*17;
            break;
        case ok_idx:
            field[0] = 'S';
            field[1] = 'e';
            field[2] = 't';
            field[3] = '\0';
            x= 55;
            y = 50;
            field_width = 3;
            break;
    }
    field[sizeof(field)-1] = '\0';
    screen.draw_string(font, x, y, field, field_width, fg, bg);
}

void rppicomidi::Clock_set_view::draw()
{
    screen.clear_canvas();
    screen.center_string(font, "Set File System", 0);
    screen.center_string(font, "Date and Time", font.height);
    screen.center_string(font, "MM/DD/YYYY HH:MM:SS",font.height*2);
    char line[30];
    sprintf(line, "%02u/%02u/%04u %02u:%02u:%02u", month, day, year, hour, minute, second);
    screen.center_string(font, line, font.height*3);
    screen.center_string(font, "Set",50);
    draw_field(item_idx, true);
}

void rppicomidi::Clock_set_view::on_left(uint32_t delta, bool is_shifted)
{
    (void)delta;
    uint8_t prev_idx = item_idx;
    if (item_idx > 0) {
        if (is_shifted) {
            if (item_idx == ok_idx) {
                item_idx = hour_idx;
            }
            else if(item_idx >= min_idx) {
                item_idx = hour_idx;
            }
            else {
                item_idx = month_idx;
            }
        }
        else {
            --item_idx;
        }
        draw_field(prev_idx, false);
        draw_field(item_idx, true);
    }
}

void rppicomidi::Clock_set_view::on_right(uint32_t delta, bool is_shifted)
{
    (void)delta;
    uint8_t prev_idx = item_idx;
    if (item_idx < ok_idx) {
        if (is_shifted) {
            if (item_idx <= year_idx) {
                item_idx = hour_idx;
            }
            else if (item_idx <= sec_idx) {
                item_idx = ok_idx;
            }
        }
        else {
            ++item_idx;
        }
        draw_field(prev_idx, false);
        draw_field(item_idx, true);
    }
}

void rppicomidi::Clock_set_view::on_increment(uint32_t delta, bool is_shifted)
{
    (void)delta;
    switch(item_idx) {
        default:
            break;
        case month_idx:
            if (month < 12) {
                month++;
            }
            break;
        case day_idx:
            if (day < Rp2040_rtc::instance().get_max_days_for_month(year, month)) {
                ++day;
            }
            break;
        case year_idx:
            if (is_shifted && year <= 9989) {
                year+=10;
            }
            else if (year < 9999) {
                ++year;
            }
            break;
        case hour_idx:
            if (is_shifted && hour <= 13)
                hour += 10;
            else if (hour < 23)
                ++hour;
            break;
        case min_idx:
            if (is_shifted && minute <= 49)
                minute += 10;
            else if (minute < 59) 
                ++minute;
            break;
        case sec_idx:
            if (is_shifted && second <= 49)
                second += 10;
            else if (second < 59)
                ++second;
            break;
    }
    draw_field(item_idx, true);
}

void rppicomidi::Clock_set_view::on_decrement(uint32_t delta, bool is_shifted)
{
    (void)delta;
    switch(item_idx) {
        default:
            break;
        case month_idx:
            if (month > 1) {
                --month;
            }
            break;
        case day_idx:
            if (day > 1) {
                --day;
            }
            break;
        case year_idx:
            if (is_shifted && year >= 2023)
                year -= 10;
            else if (year > 2022) {
                --year;
            }
            break;
        case hour_idx:
            if (is_shifted && hour >= 10)
                hour -= 10;
            else if (hour >0)
                --hour;
            break;
        case min_idx:
            if (is_shifted && minute >= 10)
                minute -= 10;
            else if (minute > 0) 
                --minute;
            break;
        case sec_idx:
            if (is_shifted && second >= 10)
                second -=10;
            else if (second > 0)
                --second;
            break;
    }
    draw_field(item_idx, true);
}

rppicomidi::View::Select_result rppicomidi::Clock_set_view::on_select(View** new_view)
{
    (void)new_view;
    if (item_idx == ok_idx) {
        uint8_t maxday = Rp2040_rtc::instance().get_max_days_for_month(year, month);
        if (day > maxday) {
            day = maxday;
            item_idx = day_idx;
            draw();
        }
        else {
            if (Rp2040_rtc::instance().set_date(year, month, day)) {
                if (Rp2040_rtc::instance().set_time(hour, minute, second)) {
                    return Select_result::exit_view;
                }
                else {
                    item_idx = hour_idx;
                }
            }
            else {
                item_idx = month_idx;
            }
        }
    }
    return Select_result::no_op;
}
