#ifndef __HC_SCHEDULE_H
#define __HC_SCHEDULE_H

#include "hc_list.h"
#include "hc_datetime.h"

namespace HiCreation
{
    /* TSchedule */
    class TSchedule
    {
        friend class TScheduleList;

    public:     // type
        enum Density
        {
            intvOnce = 0,
            intvSecondly,
            intvDaysOfWeek
        };

    public:
        TSchedule(const enum Density ADensity, const uint32_t AInterval, const uint32_t Flag = 0) :
            FDensity(ADensity), FInterval(AInterval), FFlag(Flag)
            { }
        TSchedule(TDateTime &dt) :
            FDensity(intvOnce), FInterval(dt.Timestamp())
            { }

        virtual ~TSchedule()
            { }

        enum Density Density()
            { return FDensity; }

        uint32_t Interval(void) const
            { return FInterval; }

        virtual void *ValuePtr(void) const
            { return NULL; }

    protected:
        virtual void Execute(void) = 0;

        void UpdateLastExecute(const TDateTime &dt)
        {
            int32_t diff = 0;

            switch (FDensity)
            {
            case intvOnce:
                break;
            case intvSecondly:
                /*
                if (FFlag > 0)
                {
                    uint32_t mod1 = dt % FInterval;
                    uint32_t mod2 = dt % FFlag;
                    if (mod1 != mod2)
                        diff =
                }
                */
                break;

            case intvDaysOfWeek:
                diff =  FInterval - dt % 86400;
                break;
            }

            FLastExecuteDT = dt + diff;
        }

    private:
        bool InternalExecute(const TDateTime &dt)
        {
            if (NeedExecute(dt))
            {
                Execute();
                UpdateLastExecute(dt);
                return true;
            }
            else
                return false;
        }

        bool NeedExecute(const TDateTime &dt) const
        {
            if (FInterval == 0)
                return false;
            if (FDensity == intvOnce)
                return FLastExecuteDT.IsNull() && (dt >= TDateTime(FInterval));

            int second_diff = SecondsBetween(dt, FLastExecuteDT);
            if (second_diff <= 0)   // schedule least 1 second
                return false;

            switch (FDensity)
            {
            case intvSecondly:
                return second_diff >= FInterval;

            case intvDaysOfWeek:
                return (second_diff >= 86400) && (dt.SecondOfDay() >= FInterval) && (FFlag & (1 << dt.DayOfWeek()));

            default:
                return false;
            }
        }

    protected:
        enum Density FDensity;
        uint32_t FInterval;
        uint32_t FFlag;
        TDateTime FLastExecuteDT;
    };


    class TScheduleList: public TList<TSchedule *>
    {
        typedef TList<TSchedule *> inherited;

    public:
        TScheduleList() :
            inherited()
        {
            TDateTime ts = TDateTime(TS_2000);
            if (TDateTime::Now() < ts)
                TDateTime::UpdateSystemTime(ts);
        }

        virtual ~TScheduleList()
        {
            Clear();
        }

        void Execute(void)
        {
            TDateTime dt = TDateTime::Now();

            TScheduleList::Iterator it = Begin();
            while (it != End())
            {
                if ((*it)->InternalExecute(dt) && (*it)->Density() == TSchedule::intvOnce)
                    Remove(it);
                else
                    it ++;
            }
        }

        void Clear()
        {
            TScheduleList::Iterator it = Begin();
            while (it != End())
            {
                Remove(it);
            }
        }

        virtual void Remove(Iterator &it)
        {
            TSchedule *schedule = *Extract(it);

            if (schedule)
                delete schedule;
        }

        virtual void Add(TSchedule *Schedule)
            { inherited::PushBack(Schedule); }
    };

};

#endif // __HC_SCHEDULE_H
