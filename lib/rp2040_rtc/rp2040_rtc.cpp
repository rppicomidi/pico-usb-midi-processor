#include "rp2040_rtc.h"
#include <cmath>
#include <cstring>
#include <cstdlib>

const char* const rppicomidi::Rp2040_rtc::month_name[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

uint8_t rppicomidi::Rp2040_rtc::get_day_of_the_week(uint16_t year, uint8_t month, uint8_t day)
{
    // https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
    // Keith's and Cramer's algorithm
    return (day+=month<3?year--:year-2,23*month/9+day+4+year/4-year/100+year/400)%7;
}

uint8_t rppicomidi::Rp2040_rtc::get_month_number(char* month_str)
{
    uint8_t month_num=0;
    for(; month_num<12 && strncmp(month_str, month_name[month_num], 3) != 0; month_num++) {
    }

    return month_num+1;
}


uint8_t rppicomidi::Rp2040_rtc::get_max_days_for_month(uint16_t year, uint8_t month)
{
    static const uint8_t days_in_month[] {
        31, // Jan
        28, // Feb (non leap year)
        31, // Mar
        30, // Apr
        31, // May
        30, // Jun
        31, // Jul
        31, // Aug
        30, // Sep
        31, // Oct
        30, // Nov
        31, // Dec
    };
    uint8_t max_day = 0; // an illegal day number
    if (month == 2 && (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0)) {
        max_day = 29;
    }
    else if (month <= sizeof(days_in_month) && year < 10000) {
        max_day = days_in_month[month-1];
    }
    return max_day;
}

bool rppicomidi::Rp2040_rtc::day_ok_for_month(uint16_t year, uint8_t month, uint8_t day)
{
    return day >0 && day <= get_max_days_for_month(year, month);
}


rppicomidi::Rp2040_rtc::Rp2040_rtc()
{
    char compile_date[] = __DATE__; // the compile date string: e.g. "Nov 03 2022"
    char* cdate_str = strtok(compile_date, " ");
    int8_t month = get_month_number(cdate_str);
    int8_t day = atoi(strtok(NULL, " "));
    int16_t year = atoi(strtok(NULL, " "));
    char compile_time[] = __TIME__; // the compile time string: e.g. 07:35:23
    char* ctime_str = strtok(compile_time, ":");
    int8_t hour = atoi(ctime_str);
    int8_t min = atoi(strtok(NULL, ":"));
    int8_t sec = atoi(strtok(NULL," "));
    datetime_t t = {
            .year  = year,
            .month = month,
            .day   = day,
            .dotw  = (int8_t)get_day_of_the_week(year, month, day),
            .hour  = hour,
            .min   = min,
            .sec   = sec
    };

    // Start the RTC
    rtc_init();
    rtc_set_datetime(&t);
    // clk_sys is >2000x faster than clk_rtc, so datetime is not updated immediately when rtc_set_datetime() is called.
    // tbe delay is up to 3 RTC clock cycles (which is 64us with the default clock settings)
    sleep_us(64);
}

bool rppicomidi::Rp2040_rtc::set_date(uint16_t year, uint8_t month, uint8_t day)
{
    bool success = ((year <= 9999) && (month >=1 && month <=12) && day_ok_for_month(year, month, day));
    if (success) {
        datetime_t t;
        rtc_get_datetime(&t);
        t.year = year;
        t.month = month;
        t.day = day;
        t.dotw = get_day_of_the_week(year, month, day);
        rtc_set_datetime(&t);
        sleep_us(64);
    }
    return success;
}

bool rppicomidi::Rp2040_rtc::set_time(uint8_t hour, uint8_t min, uint8_t sec)
{
    bool success = ((hour <= 24) && (min < 60) && (sec < 60));
    if (success) {
        datetime_t t;
        rtc_get_datetime(&t);
        t.hour = hour;
        t.min = min;
        t.sec = sec;
        rtc_set_datetime(&t);
        sleep_us(64);
    }
    return success;
}

void rppicomidi::Rp2040_rtc::get_date(uint16_t& year, uint8_t& month, uint8_t& day, uint8_t& dow)
{
    datetime_t t;
    rtc_get_datetime(&t);
    year = t.year;
    month = t.month;
    day = t.day;
    dow = t.dotw;
}

void rppicomidi::Rp2040_rtc::get_time(uint8_t& hour, uint8_t& min, uint8_t& sec)
{
    datetime_t t;
    rtc_get_datetime(&t);
    hour = t.hour;
    min = t.min;
    sec = t.sec;
}

const char* rppicomidi::Rp2040_rtc::get_month_name(uint8_t month_num)
{
    const char* name_ptr = nullptr;
    if (month_num > 0 && month_num <= 12) {
        name_ptr = month_name[month_num];
    }
    return name_ptr;
}

uint32_t rppicomidi::Rp2040_rtc::get_fat_date_time()
{
    uint32_t result = ~0; //error value
    datetime_t t;
    rtc_get_datetime(&t);
    uint32_t yearfield = (t.year-1980);
    if (yearfield < 127) {        
        result = (yearfield << 25) | (t.month << 21) | (t.day << 16) | (t.hour << 11) | (t.min << 5) | (t.sec / 2);
    }
    return result;
}


// For FatFs library
extern "C" uint32_t get_fattime(void)
{
    return rppicomidi::Rp2040_rtc::instance().get_fat_date_time();
}