#ifndef __HC_THREAD_H
#define __HC_THREAD_H

#include <pthread.h>

namespace HiCreation
{
    // from UltraCreation
    class TThread
    {
    private:
        static inline void *Entry(void *self)
        {
            ((TThread *)self)->Execute();
            return NULL;
        }

    public:
        TThread():
            FTerminating(true)
        {
        }

        void Start()
        {
            FTerminating = false;
            pthread_create(&FId, NULL, TThread::Entry, this);
        }

        pthread_t Id() const
        {
            return FId;
        }

        bool Terminating() const
        {
            return FTerminating;
        }

        /**
         *  for safest terminate thread: this function only set @terminating flag
         *      derived Execute should peek this flag to exit executing
         */
        void SetTerminating(void)
        {
            FTerminating = true;
        }

        /**
         *  wait for this thread exit, must called by 3party, otherwise -1(EDEADLK) was returned
         *      return 0 when successed
         */
        int WaitForTerminated(void **retval = NULL)
        {
            void *ret;
            if (retval == NULL)
                retval = &ret;

            return pthread_join(FId, retval);
        }

        int Terminate(void **retval = NULL)
        {
            SetTerminating();
            return WaitForTerminated(retval);
        }

    protected:
        virtual void Execute(void) = 0;

    protected:
        pthread_t FId;
        bool FTerminating;
    };
};

#endif // __HC_THREAD_H
