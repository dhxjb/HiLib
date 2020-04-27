#ifndef __HC_DATETIME_H
#define __HC_DATETIME_H

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#define EPOCH_2000                  ((time_t)0x386D4380)
#define TS_2000                     EPOCH_2000

namespace HiCreation
{
    class TDateTime
    {
    private:
        time_t FVal;

    public:
        TDateTime() :
            FVal(0)
            { }
        TDateTime(const time_t val) :
            FVal(val)
            { }

        static int UpdateSystemTime(TDateTime &dt)
            { return stime(&dt.FVal); }

        static TDateTime Now()
            { return time(NULL); }

        static TDateTime Null()
            { return TDateTime(); }

        bool IsNull() const
            { return 0 == FVal; }

        TDateTime DatePart() const
            { return FVal - TimePart(); }

        TDateTime TimePart() const
            { return FVal % 86400; }

        uint32_t Timestamp() const
            { return FVal; }

        uint32_t Epoch() const
            { return FVal; }

        uint32_t SecondOfMinute(void) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            return tv.tm_sec;
        }

        uint32_t SecondOfHour(void) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            return tv.tm_min * 60 + tv.tm_sec;
        }

        uint32_t SecondOfDay(void) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            return tv.tm_hour * 3600 + tv.tm_min * 60 + tv.tm_sec;
        }

        uint32_t MinuteOfHour(void) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            return tv.tm_min;
        }

        uint32_t MinuteOfDay(void) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            return tv.tm_hour * 60 + tv.tm_min;
        }

        uint32_t HourOfDay(void) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            return tv.tm_hour;
        }

        uint32_t DayOfWeek(void) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            return tv.tm_wday;
        }

        uint32_t DayOfMonth(void) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            return tv.tm_mday;
        }

        uint32_t DayOfYear(void)const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            return tv.tm_yday;
        }

    // diff
        int32_t SecondsBetween(const TDateTime &r) const
            { return FVal - r.FVal; }
        int32_t MinutesBetween(const TDateTime &r) const
            { return SecondsBetween(r) / 60; }
        int32_t HoursBetween(const TDateTime &r) const
            { return SecondsBetween(r) / 3600; }
        int32_t DaysBetween(const TDateTime &r) const
            { return SecondsBetween(r) / 86400; }

    // decode
        void Decode(struct tm &tv) const
        {
            localtime_r(&FVal, &tv);
        }

        void DecodeDate(uint16_t &year, uint16_t &mon, uint16_t &day) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            year = tv.tm_year;
            mon = tv.tm_mon;
            day = tv.tm_yday;
        }

        void DecodeTime(uint16_t &hour, uint16_t &min, uint16_t &sec) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            hour = tv.tm_hour;
            min = tv.tm_min;
            sec = tv.tm_sec;
        }

        void DecodeDateTime(uint16_t &year, uint16_t &mon, uint16_t &day, uint16_t &hour, uint16_t &min, uint16_t &sec) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            year = tv.tm_year;
            mon = tv.tm_mon;
            day = tv.tm_yday;
            hour = tv.tm_hour;
            min = tv.tm_min;
            sec = tv.tm_sec;
        }

        int DecodeString(char *str) const
        {
            struct tm tv;
            localtime_r(&FVal, &tv);

            return sprintf(str, "%d/%d/%d %02d:%02d:%02d",
                tv.tm_year + 1900, tv.tm_mon + 1, tv.tm_mday, tv.tm_hour, tv.tm_min, tv.tm_sec);
        }

    // encode
        bool Encode(const struct tm &tv)
        {
        /*
            if (tv.tm_year > 3000 || tv.tm_mon > 12 || tv.tm_mday > 31 ||
                tv.tm_hour > 24 || tv.tm_min > 59 || tv.tm_sec > 60)
            {
                return false;
            }
        */
            FVal = mktime((struct tm *)&tv);
            return FVal != (time_t)-1;
        }

        bool EncodeDate(const uint16_t year, const uint16_t mon, const uint16_t day)
        {
            struct tm tv = {0};
            // memset(&tv, 0, sizeof(tv));

            tv.tm_year = year;
            tv.tm_mon = mon;
            tv.tm_yday = day;

            return Encode(tv);
        }

        bool EncodeTime(const uint16_t hour, const uint16_t min, const uint16_t sec)
        {
            struct tm tv = {0};
            // memset(&tv, 0, sizeof(tv));

            tv.tm_hour = hour;
            tv.tm_min = min;
            tv.tm_sec = sec;

            return Encode(tv);
        }

        bool EncodeDateTime(const uint16_t year, const uint16_t mon, const uint16_t day,
            const uint16_t hour, const uint16_t min, const uint16_t sec)
        {
            struct tm tv = {0};
            // memset(&tv, 0, sizeof(tv));

            tv.tm_year = year;
            tv.tm_mon = mon;
            tv.tm_yday = day;

            tv.tm_hour = hour;
            tv.tm_min = min;
            tv.tm_sec = sec;

            return Encode(tv);
        }

    // operators
        operator time_t (void) const
            { return FVal; }

        bool operator ==(const TDateTime &r) const
            { return FVal == r.FVal; }
        bool operator ==(const time_t &r) const
            { return FVal == r; }

        bool operator !=(const TDateTime &r) const
            {return FVal != r.FVal; }
        bool operator !=(const time_t &r) const
            { return FVal != r; }

        bool operator > (const TDateTime &r) const
            { return FVal > r.FVal;}
        bool operator > (const time_t &r) const
            { return FVal > r; }

        bool operator < (const TDateTime &r) const
            { return FVal < r.FVal; }
        bool operator < (const time_t &r) const
            { return FVal < r;}

        bool operator >= (const TDateTime &r) const
            { return FVal >= r.FVal; }
        bool operator >= (const time_t &r) const
            { return FVal >= r;}

        bool operator <= (const TDateTime &r) const
            { return FVal <= r.FVal; }
        bool operator <= (const time_t &r) const
            { return FVal <= r; }
    };

    static inline
        int SecondsBetween(const TDateTime &l, const TDateTime &r)
        {
            return l.SecondsBetween(r);
        }

    static inline
        int MinutesBetween(const TDateTime &l, const TDateTime &r)
        {
            return l.MinutesBetween(r);
        }

    static inline
        int HoursBetween(const TDateTime &l, const TDateTime &r)
        {
            return l.HoursBetween(r);
        }

    static inline
        int DaysBetween(const TDateTime &l, const TDateTime &r)
        {
            return l.DaysBetween(r);
        }
};

#endif // __HC_DATETIME_H
