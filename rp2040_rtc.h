/**
 * @file rp2040_rtc.h
 * @brief this class is a wrapper for managing the RP2040 real-time clock
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
#include <cstdint>
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
namespace rppicomidi
{
class Rp2040_rtc
{
public:
    // Singleton Pattern

    /**
     * @brief Get the Instance object
     *
     * @return the singleton instance
     */
    static Rp2040_rtc& instance()
    {
        static Rp2040_rtc _instance;    // Guaranteed to be destroyed.
                                        // Instantiated on first use.
        return _instance;
    }
    Rp2040_rtc(Rp2040_rtc const&) = delete;
    void operator=(Rp2040_rtc const&) = delete;

    /**
     * @brief Construct a new Rp2040_rtc object
     */
    Rp2040_rtc();

    /**
     * @brief Set the RTC date
     * 
     * @param year the 4 digit year
     * @param month from 1-12
     * @param day from 1-31, as appropriate for the month
     * @return true if date was successfully set
     * @return false otherwise
     */
    bool set_date(uint16_t year, uint8_t month, uint8_t day);

    /**
     * @brief Set the RTC time
     * 
     * @param hour from 0-23
     * @param min from 0-59
     * @param sec from 0-59
     * @return true if time was successfully set
     * @return false otherwise
     */
    bool set_time(uint8_t hour, uint8_t min, uint8_t sec);

    /**
     * @brief Get the date from the RTC
     * 
     * @param year the 4 digit year
     * @param month from 1-12
     * @param day from 1-31, as appropriate for the month
     */
    void get_date(uint16_t& year, uint8_t& month, uint8_t& day)
    {
        uint8_t dow;
        get_date(year, month, day, dow);
    }

    /**
     * @brief Get the date from the RTC
     * 
     * @param year the 4 digit year
     * @param month from 1-12
     * @param day from 1-31, as appropriate for the month
     * @param dow the day of the week 0-6 is Sun-Sat
     */
    void get_date(uint16_t& year, uint8_t& month, uint8_t& day, uint8_t& dow);

    /**
     * @brief Get the time from the RTC
     * 
     * @param hour from 0-23
     * @param min from 0-59
     * @param sec from 0-59
     */
    void get_time(uint8_t& hour, uint8_t& min, uint8_t& sec);

    /**
     * @brief Get the 32bit date time value
     * 
     * @return uint32_t return the 32-bit date and time value
     * per the Microsoft FAT specification (August 30, 2005)
     */
    uint32_t get_32bit_date_time();

    /**
     * @brief Get the month name for the month number
     * 
     * @param month_num the month number 1-12
     * @return const char* 3-letter month name or nullptr if
     * month_num is out of range
     */
    const char* get_month_name(uint8_t month_num);

    /**
     * @brief Get the 32-bit date and time since Jan 1, 1980
     * per the Microsoft FAT Specification from August 30, 2005
     * 
     * @return uint32_t the date and time value
     */
    uint32_t get_fat_date_time();
private:
    /**
     * @brief Get the day of the week object
     * 
     * @param year the 4-digit year
     * @param month the month from 1 (January) to 12 (December)
     * @param day the day of the month from 1 to 31
     * @return uint8_t the day of the week, where Sunday is 0, Monday is 1, etc.
     */
    uint8_t get_day_of_the_week(uint16_t year, uint8_t month, uint8_t day);

    /**
     * @brief Get the month number object
     * 
     * @param month_str the month abbreviation as returned in the __DATE__ operator
     * @return uint8_t the month number, from 1 or 13 if there is an error
     */
    uint8_t get_month_number(char* month_str);

    /**
     * @brief Get the maximum number of days for the given month in the given year
     * 
     * @param year the four digit year
     * @param month the month 1-12
     * @return uint8_t the maximum number of days for that month in the given year
     * or 0 if the year or month is out of range
     */
    uint8_t get_max_days_for_month(uint16_t year, uint8_t month);

    /**
     * @brief check if the day is in range for the given month and year
     * 
     * @param year 4-digit year needed for checking leap year
     * @param month the month from 1-12
     * @param day the day from 1 to 31
     * @return true if the day is in range
     * @return false otherwsie
     */
    bool day_ok_for_month(uint16_t year, uint8_t month, uint8_t day);
    static const char* const month_name[];
};
}