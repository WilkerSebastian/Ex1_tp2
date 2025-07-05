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

} // end namespace tcii::ex

#endif // __GC_h