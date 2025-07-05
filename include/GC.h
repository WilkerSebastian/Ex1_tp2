#ifndef __GC_h
#define __GC_h

#include <new>

namespace tcii::ex
{ // begin namespace tcii::ex


//////////////////////////////////////////////////////////
//
// BlockInfo class
// =========
struct BlockInfo
{
  unsigned flag;
  unsigned size;
  BlockInfo* prev;
  BlockInfo* next;

}; // BlockInfo


//////////////////////////////////////////////////////////
//
// Allocator class
// =========
class Allocator
{
public:
  static constexpr auto minBlockSize = 16u;
  static constexpr auto heapSize = 1048576u;

  static void initialize(unsigned = heapSize);
  template <typename T> static T* allocate(unsigned = 1);
  static void free(void*);
  static void printMemoryMap();
  static void exit();

private:
  static Allocator* _instance;
  static void* _heap;              
  static BlockInfo* _free_list_head; 

  Allocator();
  ~Allocator();

  Allocator(const Allocator&) = delete;
  Allocator& operator=(const Allocator&) = delete;

}; // Allocator

// Resolvendo problema de link editor colocando o método allocate no .h
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

#endif // __GC_h