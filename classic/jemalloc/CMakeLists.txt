set(JEMALLOC_SOURCES
    jemalloc.cpp
    jemalloc_strategy.cpp
)

add_library(jemalloc_classic STATIC ${JEMALLOC_SOURCES})
target_include_directories(jemalloc_classic PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/classic/jemalloc
    ${CMAKE_SOURCE_DIR}/classic/common
)
target_compile_options(jemalloc_classic PRIVATE -O2 -march=native -Wall -Wextra -Wno-unused-parameter)

add_executable(test_jemalloc_classic test/test_jemalloc.cpp)
target_link_libraries(test_jemalloc_classic PRIVATE jemalloc_classic pthread)
target_compile_options(test_jemalloc_classic PRIVATE -O2 -march=native)
add_test(NAME jemalloc_classic COMMAND test_jemalloc_classic)
