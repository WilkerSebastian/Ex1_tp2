#include "GC.h"
#include <iostream>
#include <cstdint>

namespace tcii::ex
{ // begin namespace tcii::ex


//////////////////////////////////////////////////////////
//
// Allocator implementation
// =========
Allocator* Allocator::_instance = nullptr;
void* Allocator::_heap = nullptr;
BlockInfo* Allocator::_free_list_head = nullptr;
unsigned Allocator::_actualHeapSize = 0;

Allocator::Allocator() {}

Allocator::~Allocator()
{

    if (_heap != nullptr) 
        delete[] static_cast<char*>(_heap);

}

void Allocator::initialize(unsigned size)
{

    if (_instance != nullptr) 
        return;

    _instance = new Allocator();

    _actualHeapSize = size;

    _heap = new char[_actualHeapSize];

    if (_heap == nullptr) 
        throw std::bad_alloc();

    BlockInfo* initialBlock = static_cast<BlockInfo*>(_heap);

    const unsigned usableSize = _actualHeapSize - headerFooterSize;

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

void Allocator::free(void* p)
{
  
    if (p == nullptr) 
        return;

    BlockInfo* blockToFree = static_cast<BlockInfo*>(p) - 1;

    if (blockToFree->flag == 0) 
        return;

    blockToFree->flag = 0;

    BlockInfo* footerToFree = reinterpret_cast<BlockInfo*>(
        (char*)(blockToFree + 1) + blockToFree->size
    );

    footerToFree->flag = 0;
    footerToFree->prev = blockToFree;

    BlockInfo* leftNeighbor = nullptr;

    if (reinterpret_cast<uintptr_t>(blockToFree) > reinterpret_cast<uintptr_t>(_heap)) 
    {

        BlockInfo* prevFooter = reinterpret_cast<BlockInfo*>(
            (char*)blockToFree - sizeof(BlockInfo)
        );
        
        if (prevFooter->flag == 0)
            leftNeighbor = prevFooter->prev;

    }

    BlockInfo* rightNeighbor = nullptr;

    BlockInfo* nextHeader = reinterpret_cast<BlockInfo*>(
        (char*)footerToFree + sizeof(BlockInfo)
    );

    if (reinterpret_cast<uintptr_t>(nextHeader) < reinterpret_cast<uintptr_t>(_heap) + _actualHeapSize) 
    {

        if (nextHeader->flag == 0) 
            rightNeighbor = nextHeader;
        
    }

    if (leftNeighbor != nullptr && rightNeighbor != nullptr) 
    {
        
        leftNeighbor->size += blockToFree->size + rightNeighbor->size + (2 * headerFooterSize);

        rightNeighbor->prev->next = rightNeighbor->next;
        rightNeighbor->next->prev = rightNeighbor->prev;

        if (_free_list_head == rightNeighbor || _free_list_head == blockToFree) 
            _free_list_head = leftNeighbor;

    } 
    else if (leftNeighbor != nullptr) 
    {
        
        leftNeighbor->size += blockToFree->size + headerFooterSize;

        if (_free_list_head == blockToFree) 
            _free_list_head = leftNeighbor;
    
    } 
    else if (rightNeighbor != nullptr)
    {
        
        blockToFree->size += rightNeighbor->size + headerFooterSize;
        blockToFree->prev = rightNeighbor->prev;
        blockToFree->next = rightNeighbor->next;
        blockToFree->prev->next = blockToFree;
        blockToFree->next->prev = blockToFree;

        if (_free_list_head == rightNeighbor) 
            _free_list_head = blockToFree;
        
    } 
    else 
    {
        
        if (_free_list_head == nullptr) 
        {

            _free_list_head = blockToFree;

            blockToFree->next = blockToFree;
            blockToFree->prev = blockToFree;

            return;

        } 

        blockToFree->next = _free_list_head;
        blockToFree->prev = _free_list_head->prev;

        _free_list_head->prev->next = blockToFree;
        _free_list_head->prev = blockToFree;
        
    }
    
    BlockInfo* finalBlock = leftNeighbor ? leftNeighbor : blockToFree;

    BlockInfo* finalFooter = reinterpret_cast<BlockInfo*>(
        (char*)(finalBlock + 1) + finalBlock->size
    );

    finalFooter->flag = 0;

    finalFooter->prev = finalBlock;

}

void Allocator::printMemoryMap()
{

    std::cout << "--- Memory Map ---\n";

    if (_heap == nullptr || _actualHeapSize == 0) {

        std::cout << "Heap not initialized or is empty.\n";
        return;

    }

    char* current_p = static_cast<char*>(_heap);
    unsigned total_free_memory = 0;
    const uintptr_t heap_end = reinterpret_cast<uintptr_t>(_heap) + _actualHeapSize;

    while (reinterpret_cast<uintptr_t>(current_p) < heap_end) {

        BlockInfo* block = reinterpret_cast<BlockInfo*>(current_p);

        std::cout << "Block at " << static_cast<void*>(block)
                  << "\t| Flag: " << (block->flag == 0 ? "FREE  " : "ALLOC ")
                  << "\t| Size: " << block->size << " bytes\n";

        if (block->flag == 0) 
            total_free_memory += block->size;
        
        current_p += headerFooterSize + block->size;

    }

    std::cout << std::string(50, '-') << '\n';
    std::cout << "Total available memory: " << total_free_memory << " bytes\n";
    std::cout << std::string(50, '-') << "\n\n";

}

void Allocator::exit()
{

    if (_instance == nullptr)
        return;
    
    delete _instance; 
    _instance = nullptr;

}

} // end namespace tcii::ex

