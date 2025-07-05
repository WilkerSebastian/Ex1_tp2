#include "GC.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <cassert>
#include <vector>

using namespace tcii::ex;

inline void printTestHeader(const std::string& title) {

    std::cout << "\n\n" << std::string(70, '=') << "\n";
    std::cout << "TEST: " << title << "\n";
    std::cout << std::string(70, '-') << "\n";

}

inline void testSimpleAllocationAndFree() {

    printTestHeader("Simple Allocation & Free");

    std::cout << "Initializing allocator\n";

    Allocator::initialize();
    Allocator::printMemoryMap();

    std::cout << "Allocating block p1 of 128 bytes\n";

    int* p1 = Allocator::allocate<int>(128 / sizeof(int));

    assert(p1 != nullptr);

    Allocator::printMemoryMap();

    std::cout << "Allocating block p2 of 256 bytes\n";

    char* p2 = Allocator::allocate<char>(256);

    assert(p2 != nullptr);

    Allocator::printMemoryMap();

    std::cout << "Freeing block p1\n";

    Allocator::free(p1);
    Allocator::printMemoryMap();

    std::cout << "Freeing block p2\n";

    Allocator::free(p2);
    Allocator::printMemoryMap();

    std::cout << "Re-allocating a larger block p3\n";

    double* p3 = Allocator::allocate<double>(512 / sizeof(double));

    assert(p3 != nullptr);

    Allocator::printMemoryMap();

    Allocator::exit();

}

inline void testCoalescing() {

    printTestHeader("Block Coalescing");

    Allocator::initialize();

    std::cout << "Setting up blocks for coalescing tests\n";
    
    auto p1 = Allocator::allocate<char>(100);
    auto p2 = Allocator::allocate<char>(100);
    auto p3 = Allocator::allocate<char>(100);
    auto p4 = Allocator::allocate<char>(100);

    Allocator::printMemoryMap();

    std::cout << "--- Test 1: Merging with right neighbor ---\n";
    std::cout << "Freeing p3, then p2\n";

    Allocator::free(p3);
    Allocator::printMemoryMap();

    Allocator::free(p2);
    Allocator::printMemoryMap();

    std::cout << "--- Test 2: Merging with left neighbor ---\n";
    std::cout << "Allocating p2 again, then freeing p1, then p2\n";

    p2 = Allocator::allocate<char>(100);

    Allocator::printMemoryMap();

    Allocator::free(p1);
    Allocator::printMemoryMap();

    Allocator::free(p2); 
    Allocator::printMemoryMap();

    std::cout << "--- Test 3: Merging with both neighbors ---\n";
    std::cout << "Allocating p1, p3. Then freeing p2, p4, then p3\n";

    p1 = Allocator::allocate<char>(100);
    p3 = Allocator::allocate<char>(100);
    
    Allocator::printMemoryMap();

    Allocator::free(p2);
    Allocator::free(p4);
    Allocator::printMemoryMap(); 
    
    Allocator::free(p3); 
    Allocator::printMemoryMap();

    std::cout << "--- Test 4: No neighbors to merge ---\n";
    std::cout << "Freeing p1\n";

    Allocator::free(p1);
    Allocator::printMemoryMap();

    Allocator::exit();

}

inline void testEdgeCases() {

    printTestHeader("Edge Cases & Error Handling");
    Allocator::initialize();

    std::cout << "Testing freeing a null pointer\n";

    Allocator::free(nullptr);

    Allocator::printMemoryMap();

    std::cout << "Testing allocation failure...\n";

    try {
        
        Allocator::allocate<char>(Allocator::heapSize * 2);

        assert(false); // Não deve chegar aqui

    } catch (const std::bad_alloc& e) {
        std::cout << "Caught expected exception: " << e.what() << "\n";
    }
    
    Allocator::printMemoryMap();

    std::cout << "Testing allocating nearly all memory\n";
    
    char* big_block = Allocator::allocate<char>(1048000);
    
    assert(big_block != nullptr);
    
    Allocator::printMemoryMap();
    
    std::cout << "Freeing the large block\n";
    
    Allocator::free(big_block);
    Allocator::printMemoryMap();

    Allocator::exit();

}


inline void
allocatorTest()
{

  testSimpleAllocationAndFree();

  testCoalescing();

  testEdgeCases();

}

//
// Main function
//
int
main()
{
  allocatorTest();
  return 0;
}

