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

void Allocator::free(void* p)
{
  
    if (p == nullptr)
        return;

    BlockInfo* blockToFree = static_cast<BlockInfo*>(p) - 1;

    if (blockToFree->flag == 0)
        return;
    

    BlockInfo* leftNeighbor = nullptr;
    BlockInfo* rightNeighbor = nullptr;

    const unsigned headerFooterSize = sizeof(BlockInfo);

    BlockInfo* prevFooter = reinterpret_cast<BlockInfo*>(
        reinterpret_cast<char*>(blockToFree) - headerFooterSize
    );
   
    
    if (reinterpret_cast<void*>(prevFooter) >= _heap)
    {

        if (prevFooter->flag == 0) 
            leftNeighbor = prevFooter->prev; 
        
    }

    BlockInfo* nextHeader = reinterpret_cast<BlockInfo*>(
        reinterpret_cast<char*>(blockToFree + 1) + blockToFree->size
    );

    
    if (reinterpret_cast<void*>(nextHeader)<static_cast<char*>(_heap) + heapSize)
    {

        if (nextHeader->flag == 0) 
            rightNeighbor = nextHeader;
        
    }

    if (leftNeighbor == nullptr && rightNeighbor == nullptr) 
    {

        blockToFree->flag = 0;
        blockToFree->next = _free_list_head;
        blockToFree->prev = _free_list_head->prev;
        _free_list_head->prev->next = blockToFree;
        _free_list_head->prev = blockToFree;

    }
    else if (leftNeighbor != nullptr && rightNeighbor == nullptr)
    {
        
        leftNeighbor->size += blockToFree->size + (headerFooterSize * 2);

        BlockInfo* newFooter = reinterpret_cast<BlockInfo*>(
            reinterpret_cast<char*>(leftNeighbor + 1) + leftNeighbor->size
        );

        newFooter->flag = 0;
        newFooter->prev = leftNeighbor;

    }
    else if (leftNeighbor == nullptr && rightNeighbor != nullptr) 
    {
        
        rightNeighbor->prev->next = rightNeighbor->next;
        rightNeighbor->next->prev = rightNeighbor->prev;

        blockToFree->size += rightNeighbor->size + (headerFooterSize * 2);
        blockToFree->flag = 0;
        
        BlockInfo* newFooter = reinterpret_cast<BlockInfo*>(
            reinterpret_cast<char*>(blockToFree + 1) + blockToFree->size
        );

        newFooter->flag = 0;
        newFooter->prev = blockToFree;

        blockToFree->next = _free_list_head;
        blockToFree->prev = _free_list_head->prev;
        _free_list_head->prev->next = blockToFree;
        _free_list_head->prev = blockToFree;

    }
    else 
    {
        
        rightNeighbor->prev->next = rightNeighbor->next;
        rightNeighbor->next->prev = rightNeighbor->prev;

        leftNeighbor->size += blockToFree->size + rightNeighbor->size + (headerFooterSize * 4);

        BlockInfo* newFooter = reinterpret_cast<BlockInfo*>(
            reinterpret_cast<char*>(leftNeighbor + 1) + leftNeighbor->size
        );

        newFooter->flag = 0;
        newFooter->prev = leftNeighbor;

    }

}

} // end namespace tcii::ex

