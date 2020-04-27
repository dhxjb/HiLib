#ifndef __HC_OBSERVER_H__
#define __HC_OBSERVER_H__

#include "hc_list.h"

namespace HiCreation
{
    class IObserver
    {
    public:
        virtual void Update(void *arg) = 0;
    };

    class IObservable
    {
    public:
        virtual void AddObserver(IObserver *observer) = 0;
        virtual void DeleteObserver(IObserver *observer) = 0;
        virtual void Notify(void *arg);
    };

    class TObservable : public IObservable
    {
    public:
        virtual void AddObserver(IObserver *observer)
            { FObservers.PushBack(observer); }

        virtual void DeleteObserver(IObserver *observer)
        {
            TList<IObserver *>::Iterator iter = FObservers.Begin();
            while (iter != FObservers.End())
            {
                if ((*iter) == observer)
                {
                    FObservers.Extract(iter);
                    break;
                }
                iter++;
            }
        }

        virtual void Notify(void *arg)
        {
            if (FObservers.IsEmpty())
                return;

            TList<IObserver *>::Iterator iter;
            for (iter = FObservers.Begin(); iter != FObservers.End(); iter++)
                (*iter)->Update(arg);
        }

    protected:
        TList<IObserver *> FObservers;
    };
};

#endif