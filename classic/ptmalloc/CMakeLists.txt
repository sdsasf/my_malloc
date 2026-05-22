# ptmalloc classic allocator — glibc-style reproduction

set(PTMALLOC_SOURCES
    arena.cpp
    arena_manager.cpp
    bins.cpp
    coalesce.cpp
    heap.cpp
    ptmalloc.cpp
    ptmalloc_strategy.cpp
    sys_memory.cpp
    tcache.cpp
)

add_library(ptmalloc_classic STATIC ${PTMALLOC_SOURCES})
target_include_directories(ptmalloc_classic PUBLIC
    ${CMAKE_SOURCE_DIR}/include         # for strategy.h
    ${CMAKE_SOURCE_DIR}/classic/ptmalloc
    ${CMAKE_SOURCE_DIR}/classic/common
)
target_compile_options(ptmalloc_classic PRIVATE -O2 -march=native -Wall -Wextra -Wno-unused-parameter)

# Test executable
add_executable(test_ptmalloc_classic test/test_ptmalloc.cpp)
target_link_libraries(test_ptmalloc_classic PRIVATE ptmalloc_classic pthread)
target_compile_options(test_ptmalloc_classic PRIVATE -O2 -march=native)
add_test(NAME ptmalloc_classic COMMAND test_ptmalloc_classic)
