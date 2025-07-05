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
}


} // end namespace tcii::ex

