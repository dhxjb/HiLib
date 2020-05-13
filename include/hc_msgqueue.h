#ifndef __HC_MSG_QUEUE_H__
#define __HC_MSG_QUEUE_H__

#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <deque>
#include <pthread.h>
#include <semaphore.h>

#include "hc_thread.h"

namespace HiCreation
{
    template<typename T, uint8_t CAPACITY>
    class TMsgQueue
    {
    public:
        TMsgQueue()
        {
            pthread_mutex_init(&FMutex, NULL);
        }

        virtual ~TMsgQueue()
        {
            pthread_mutex_destroy(&FMutex);
        }

        size_t Capacity() { return CAPACITY; }
        size_t Size() 
        {
            size_t ret;
            pthread_mutex_lock(&FMutex);
            ret = FQueue.size();
            pthread_mutex_unlock(&FMutex);
            return ret;
        }

        void Clear()
        {
            T *temp;
            pthread_mutex_lock(&FMutex);
            while (! FQueue.empty())
            {
                temp = FQueue.front();
                delete temp;
                FQueue.pop_front();
            }
            pthread_mutex_unlock(&FMutex);
        }
        
        int Post(T *t)
        {
            int ret = 0;
            pthread_mutex_lock(&FMutex);
            if (FQueue.size() < CAPACITY)
                FQueue.push_back(t);
            else
                ret = -ENOMEM;
            pthread_mutex_unlock(&FMutex);
            return ret;
        }

        T* Peek()
        {
            T *t;
            pthread_mutex_lock(&FMutex);
            if (! FQueue.empty())
            {
                t = FQueue.front();
                FQueue.pop_front();
            }
            else
                t = NULL;
            pthread_mutex_unlock(&FMutex);
            return t;
        }
    private:
        std::deque<T *> FQueue;
        pthread_mutex_t FMutex;
    };

    template<typename T, uint8_t CAPACITY>
    class TMsgQueueThread : protected TMsgQueue<T, CAPACITY>, protected TThread
    {
        typedef TMsgQueue<T, CAPACITY> inherited;
    public:
        TMsgQueueThread()
        {
            sem_init(&FMsgSem, 0, 0);
        }

        virtual ~TMsgQueueThread()
        {
            this->Stop();
            sem_destroy(&FMsgSem);
        }

        virtual int Post(T *t)
        {
            int ret;
            if ((ret = inherited::Post(t)) == 0)
                sem_post(&FMsgSem);
            return ret;
        }

        virtual int Start()
        {
            TThread::Start();
            return 0;
        }

        virtual void Stop()
        {
            TThread::SetTerminating();
        }

    protected:
        T* Peek()
        {
            sem_wait(&FMsgSem);
            return inherited::Peek();
        }

        virtual void Execute()
        {
            T *temp;
            while(! TThread::Terminating())
            {
                temp = Peek();
                if (temp)
                {
                    OnMsgReceived(temp);
                    delete temp;
                }
                else
                    printf("peek msg err!\n");
            }
        }

        virtual void OnMsgReceived(T *t) = 0;

    private:
        sem_t FMsgSem;
    };
};


#endif