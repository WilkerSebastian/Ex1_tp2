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

} // end namespace tcii::ex

