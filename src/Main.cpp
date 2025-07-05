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

inline void
allocatorTest()
{

    printTestHeader("Allocator Initialization 10KB Heap");

    Allocator::initialize(10240);

    Allocator::printMemoryMap();

    printTestHeader("Simple Allocation & Coalescing Test");

    std::cout << "1) Allocating p1(1024), p2(2048), p3(1024)...\n";

    char* p1 = Allocator::allocate<char>(1024);
    char* p2 = Allocator::allocate<char>(2048);
    char* p3 = Allocator::allocate<char>(1024);

    assert(p1 && p2 && p3);

    Allocator::printMemoryMap();

    std::cout << "2) Freeing p2\n";

    Allocator::free(p2);
    Allocator::printMemoryMap();

    std::cout << "3) Freeing p1\n";

    Allocator::free(p1);
    Allocator::printMemoryMap();

    std::cout << "4) Freeing p3\n";

    Allocator::free(p3);
    Allocator::printMemoryMap();

    printTestHeader("Edge Cases and Error Handling");
    
    std::cout << "5) Testing allocation failure by requesting too much memory\n";

    try {
        
        Allocator::allocate<char>(20000); 

        assert(false); // não deve executar 

    } catch (const std::bad_alloc& e) {
        std::cout << "OK. Caught expected exception: " << e.what() << "\n";
    }

    std::cout << "\nFinal cleanup\n";

    Allocator::exit();

    std::cout << "Tests finished\n";

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

