set(MIMALLOC_SOURCES
    mimalloc.cpp
    mimalloc_strategy.cpp
    os.cpp
    segment.cpp
)

add_library(mimalloc_classic STATIC ${MIMALLOC_SOURCES})
target_include_directories(mimalloc_classic PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/classic/mimalloc
    ${CMAKE_SOURCE_DIR}/classic/common
)
target_compile_options(mimalloc_classic PRIVATE -O2 -march=native -Wall -Wextra -Wno-unused-parameter)

add_executable(test_mimalloc_classic test/test_mimalloc.cpp)
target_link_libraries(test_mimalloc_classic PRIVATE mimalloc_classic pthread)
target_compile_options(test_mimalloc_classic PRIVATE -O2 -march=native)
add_test(NAME mimalloc_classic COMMAND test_mimalloc_classic)
