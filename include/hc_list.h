#ifndef __HC_LIST_H
#define __HC_LIST_H

#ifndef NULL
#include <cstddef>
#endif

namespace HiCreation
{
    //from UltraCreation List
    template <typename element_t>
    class TList
    {
    private:
        struct list_item_s
        {
            element_t element;
            struct list_item_s *linked_next;
        };
        typedef struct list_item_s* item_ptr_t;
        typedef struct list_item_s** item_iter_t;

    public:
        typedef element_t * element_ptr_t;
        typedef element_t & element_ref_t;

        class Iterator
        {
        private:
            friend class TList<element_t>;

            Iterator(item_iter_t iter):
                FItemIter(iter)
            {}

        public:
            Iterator():
                FItemIter(NULL)
            {}

            Iterator & operator =(const Iterator &r)
            {
                FItemIter = r.FItemIter;
                return *this;
            }

            Iterator & operator ++(int)
            {
                if (FItemIter)
                    FItemIter = &(*FItemIter)->linked_next;

                return *this;
            }

            bool operator ==(const Iterator &r) const
            {
                return *FItemIter == *(r.FItemIter);
            }

            bool operator !=(const Iterator &r) const
            {
                return *FItemIter != *(r.FItemIter);
            }

            element_ref_t operator *()
            {
                return (*FItemIter)->element;
            }

            element_ptr_t operator ->()
            {
                return &(*FItemIter)->element;
            }

            operator element_ptr_t() const
            {
                return operator ->();
            }

            operator item_ptr_t() const
            {
                return *FItemIter;
            }

            operator item_iter_t() const
            {
                return FItemIter;
            }

        private:
            item_iter_t FItemIter;
        };

    public:
        static element_ptr_t Allocate(void)
        {
            return (element_ptr_t)new struct list_item_s;
        }

        static void Release(element_ptr_t ptr)
        {
            delete (struct list_item_s *)ptr;
        }

        TList():
            FCount(0),
            FEntry(NULL), FExtry(&FEntry)
        {}

        ~TList()
        {
            Clear();
        }

        int Size() { return FCount; }

        bool IsEmpty(void) const
        {
            return NULL == FEntry;
        }

        void Clear()
        {
            while (true)
            {
                element_ptr_t ptr = Pop();

                if (ptr)
                    Release(ptr);
                else
                    break;
            }
        }

        void PushFront(element_ptr_t ptr)
        {
            if (NULL == FEntry)
                FExtry = &(reinterpret_cast<item_ptr_t>(ptr)->linked_next);

            reinterpret_cast<item_ptr_t>(ptr)->linked_next = FEntry;
            FEntry = reinterpret_cast<item_ptr_t>(ptr);

            FCount++;
        }

        void PushFront(const element_ref_t ref)
        {
            element_ptr_t ptr = Allocate();
            *ptr = ref;
            PushFront(ptr);
        }

        void PushBack(const element_ptr_t ptr)
        {
            reinterpret_cast<item_ptr_t>(ptr)->linked_next = NULL;

            *FExtry = reinterpret_cast<item_ptr_t>(ptr);
            FExtry = &reinterpret_cast<item_ptr_t>(ptr)->linked_next;

            FCount++;
        }

        void PushBack(const element_ref_t ref)
        {
            element_ptr_t ptr = Allocate();
            *ptr = ref;
            PushBack(ptr);
        }

	 /* pop a node, but need Release manually */
        element_ptr_t Pop(void)
        {
            item_ptr_t ptr = FEntry;
            if (ptr)
            {
                FEntry = ptr->linked_next;
                FCount--;

                if (NULL == FEntry)
                    FExtry = &FEntry;
            }
            return (element_ptr_t)ptr;
        }

        bool Pop(element_ref_t ref)
        {
            element_ptr_t ptr = Pop();
            if (ptr)
            {
                ref = *ptr;
                Release(ptr);
                FCount--;
                return true;
            }
            else
                return false;
        }

        Iterator Begin()
        {
            return Iterator(&FEntry);
        }

        Iterator End()
        {
            return Iterator(FExtry);
        }

        element_ptr_t Extract(Iterator &it)
        {
            item_iter_t node_iter = it;
            item_ptr_t item_ptr = *node_iter;

            if (item_ptr)
            {
                *node_iter = item_ptr->linked_next;

                if (NULL == FEntry)
                    FExtry = &FEntry;
                else if (FExtry == &item_ptr->linked_next)
                    FExtry = node_iter;
            }

            return (element_ptr_t)item_ptr;
        }

    private:
        int FCount;
        item_ptr_t FEntry;
        item_iter_t FExtry;
    };
};

#endif // __HC_LIST_H
