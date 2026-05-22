set(TCMALLOC_SOURCES
    central_freelist.cpp
    page_heap.cpp
    page_map.cpp
    tcmalloc.cpp
    tcmalloc_strategy.cpp
    thread_cache.cpp
)

add_library(tcmalloc_classic STATIC ${TCMALLOC_SOURCES})
target_include_directories(tcmalloc_classic PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/classic/tcmalloc
    ${CMAKE_SOURCE_DIR}/classic/common
)
target_compile_options(tcmalloc_classic PRIVATE -O2 -march=native -Wall -Wextra -Wno-unused-parameter)

add_executable(test_tcmalloc_classic test/test_tcmalloc.cpp)
target_link_libraries(test_tcmalloc_classic PRIVATE tcmalloc_classic pthread)
target_compile_options(test_tcmalloc_classic PRIVATE -O2 -march=native)
add_test(NAME tcmalloc_classic COMMAND test_tcmalloc_classic)
