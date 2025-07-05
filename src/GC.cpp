#include "GC.h"

namespace tcii::ex
{ // begin namespace tcii::ex


//////////////////////////////////////////////////////////
//
// Allocator implementation
// =========
Allocator* Allocator::_instance = nullptr;
void* Allocator::_heap = nullptr;
BlockInfo* Allocator::_free_list_head = nullptr;

Allocator::Allocator()
{
  _heap = nullptr;
  _free_list_head = nullptr;
}

Allocator::~Allocator()
{

    if (_heap != nullptr)
    {
        delete[] static_cast<char*>(_heap);
        _heap = nullptr;
    }

}

void Allocator::initialize(unsigned size)
{

    if (_instance != nullptr)
    return;

    _instance = new Allocator();

    _heap = new char[size];

    if (_heap == nullptr)
        throw std::bad_alloc();

    BlockInfo* initialBlock = static_cast<BlockInfo*>(_heap);

    const unsigned headerFooterSize = sizeof(BlockInfo) * 2;
    const unsigned usableSize = size - headerFooterSize;

    initialBlock->flag = 0;
    initialBlock->size = usableSize;
    initialBlock->prev = initialBlock; 
    initialBlock->next = initialBlock;

    BlockInfo* footer = reinterpret_cast<BlockInfo*>(
        reinterpret_cast<char*>(initialBlock + 1) + usableSize
    );

    footer->flag = 0; 
    footer->prev = initialBlock;

    _free_list_head = initialBlock;

}

template <typename T>
T* Allocator::allocate(unsigned n)
{
    
    unsigned requestedSize = sizeof(T) * n;

    if (requestedSize < minBlockSize)
        requestedSize = minBlockSize;

    BlockInfo* current = _free_list_head;
    
    if (current == nullptr) 
        throw std::bad_alloc();

    do {
        
        if (current->size >= requestedSize)
        {
            
            const unsigned headerFooterSize = sizeof(BlockInfo) * 2;
            const unsigned remainingSize = current->size - requestedSize;

            if (remainingSize <= headerFooterSize + minBlockSize)
            {
                
                current->prev->next = current->next;
                current->next->prev = current->prev;

                if (_free_list_head == current) 
                    _free_list_head = (current->next == current) ? nullptr : current->next;
                
                current->flag = 1;
                
                BlockInfo* footer = reinterpret_cast<BlockInfo*>(
                    reinterpret_cast<char*>(current + 1) + current->size
                );

                footer->flag = 1;

                return static_cast<T*>(
                    static_cast<void*>(current + 1)
                );

            }
            else
            {
                
                current->size = remainingSize - headerFooterSize;

                BlockInfo* newFreeFooter = reinterpret_cast<BlockInfo*>(
                    reinterpret_cast<char*>(current + 1) + current->size
                );
                
                newFreeFooter->flag = 0;
                newFreeFooter->prev = current;

                BlockInfo* newAllocatedBlock = reinterpret_cast<BlockInfo*>(
                    reinterpret_cast<char*>(newFreeFooter) + sizeof(BlockInfo)
                );

                newAllocatedBlock->flag = 1;
                newAllocatedBlock->size = requestedSize;

                BlockInfo* newAllocatedFooter = reinterpret_cast<BlockInfo*>(
                    reinterpret_cast<char*>(newAllocatedBlock + 1) + newAllocatedBlock->size
                );
                newAllocatedFooter->flag = 1;

                return static_cast<T*>(
                    static_cast<void*>(newAllocatedBlock + 1)
                );

            }

        }

        current = current->next;

    } while (current != _free_list_head);

    throw std::bad_alloc();

}

} // end namespace tcii::ex

