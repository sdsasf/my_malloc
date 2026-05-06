cmake_minimum_required(VERSION 3.14)
project(my_ptmalloc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build shared library for LD_PRELOAD
file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(my_ptmalloc SHARED ${SOURCES})
target_include_directories(my_ptmalloc PUBLIC include)
target_link_libraries(my_ptmalloc PRIVATE pthread dl)
target_compile_options(my_ptmalloc PRIVATE -O2 -march=native -Wall -Wextra -Wno-unused-parameter)

# Build static library for tests
add_library(my_ptmalloc_static STATIC ${SOURCES})
target_include_directories(my_ptmalloc_static PUBLIC include)
target_link_libraries(my_ptmalloc_static PRIVATE pthread dl)
target_compile_options(my_ptmalloc_static PRIVATE -O2 -march=native -Wall -Wextra -Wno-unused-parameter)

# Tests
enable_testing()

# Basic test
add_executable(test_basic test/test_basic.cpp)
target_link_libraries(test_basic PRIVATE my_ptmalloc_static pthread)
target_compile_options(test_basic PRIVATE -O2 -march=native)
add_test(NAME basic COMMAND test_basic)

# Tcache test
add_executable(test_tcache test/test_tcache.cpp)
target_link_libraries(test_tcache PRIVATE my_ptmalloc_static pthread)
target_compile_options(test_tcache PRIVATE -O2 -march=native)
add_test(NAME tcache COMMAND test_tcache)

# Stress test
add_executable(test_stress test/test_stress.cpp)
target_link_libraries(test_stress PRIVATE my_ptmalloc_static pthread)
target_compile_options(test_stress PRIVATE -O2 -march=native)
add_test(NAME stress COMMAND test_stress)

add_executable(test_adaptive_control test/test_adaptive_control.cpp)
target_link_libraries(test_adaptive_control PRIVATE my_ptmalloc_static pthread)
target_compile_options(test_adaptive_control PRIVATE -O2 -march=native)
foreach(profile balanced low_latency low_rss large_heavy cross_thread)
  add_test(NAME adaptive_control_${profile}
           COMMAND ${CMAKE_COMMAND} -E env
           MY_MALLOC_MODE=adaptive
           MY_MALLOC_ADAPTIVE_PROFILE=${profile}
           $<TARGET_FILE:test_adaptive_control>)
endforeach()

add_executable(test_debug test/test_debug.cpp)
target_link_libraries(test_debug PRIVATE my_ptmalloc_static pthread)
target_compile_options(test_debug PRIVATE -O0 -g)

add_executable(test_single test/test_single.cpp)
target_link_libraries(test_single PRIVATE my_ptmalloc_static pthread)
target_compile_options(test_single PRIVATE -O0 -g)

# Performance benchmark
add_executable(test_perf test/test_perf.cpp)
target_link_libraries(test_perf PRIVATE my_ptmalloc_static pthread)
target_compile_options(test_perf PRIVATE -O2 -march=native)

# Heap inspector tool
add_executable(heap_inspect tools/heap_inspect.cpp)
target_link_libraries(heap_inspect PRIVATE my_ptmalloc_static)
target_compile_options(heap_inspect PRIVATE -O2 -march=native)

# Strategy validation and benchmark tools
add_executable(allocator_validate tools/allocator_validate.cpp)
target_include_directories(allocator_validate PRIVATE include tools)
target_link_libraries(allocator_validate PRIVATE my_ptmalloc_static pthread dl)
target_compile_options(allocator_validate PRIVATE -O2 -march=native)

foreach(strategy hybrid ptmalloc tcmalloc_like jemalloc_like mimalloc_like adaptive adaptive_demo libc)
  add_test(NAME validate_${strategy}
           COMMAND allocator_validate --strategy ${strategy})
endforeach()
add_test(NAME validate_demo_all COMMAND allocator_validate --strategy demo_all)

add_test(NAME validate_adaptive_ucb1_static
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_POLICY=ucb1
         MY_MALLOC_ADAPTIVE_PARAM_POLICY=static
         $<TARGET_FILE:allocator_validate> --strategy adaptive)
add_test(NAME validate_adaptive_ucb1_heuristic
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_POLICY=ucb1
         MY_MALLOC_ADAPTIVE_PARAM_POLICY=heuristic
         $<TARGET_FILE:allocator_validate> --strategy adaptive)
add_test(NAME validate_adaptive_coordinate
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_PARAM_POLICY=coordinate_bandit
         $<TARGET_FILE:allocator_validate> --strategy adaptive)

add_executable(bench_runner tools/bench_runner.cpp)
target_include_directories(bench_runner PRIVATE include tools)
target_link_libraries(bench_runner PRIVATE my_ptmalloc_static pthread dl)
target_compile_options(bench_runner PRIVATE -O2 -march=native)

# Example plugin strategy for users to copy.
add_library(example_counting_strategy MODULE plugins/counting_malloc_strategy.cpp)
target_include_directories(example_counting_strategy PRIVATE include)
target_compile_options(example_counting_strategy PRIVATE -O2 -march=native -Wall -Wextra)
set_target_properties(example_counting_strategy PROPERTIES
    PREFIX ""
    OUTPUT_NAME "libcounting_malloc_strategy")

# Comparison benchmarks: my_ptmalloc vs system malloc
add_executable(bench_my test/bench_compare.cpp)
target_link_libraries(bench_my PRIVATE my_ptmalloc_static pthread)
target_compile_options(bench_my PRIVATE -O2 -march=native -DUSE_MY_MALLOC)

add_executable(bench_sys test/bench_compare.cpp)
target_link_libraries(bench_sys PRIVATE pthread)
target_compile_options(bench_sys PRIVATE -O2 -march=native)

add_test(NAME preload_adaptive
         COMMAND ${CMAKE_COMMAND} -E env
         LD_PRELOAD=$<TARGET_FILE:my_ptmalloc>
         MY_MALLOC_MODE=adaptive
         $<TARGET_FILE:test_basic>)
add_test(NAME preload_adaptive_ucb1
         COMMAND ${CMAKE_COMMAND} -E env
         LD_PRELOAD=$<TARGET_FILE:my_ptmalloc>
         MY_MALLOC_MODE=adaptive
         MY_MALLOC_ADAPTIVE_POLICY=ucb1
         $<TARGET_FILE:test_basic>)
add_test(NAME preload_adaptive_static
         COMMAND ${CMAKE_COMMAND} -E env
         LD_PRELOAD=$<TARGET_FILE:my_ptmalloc>
         MY_MALLOC_MODE=adaptive
         MY_MALLOC_ADAPTIVE_PARAM_POLICY=static
         $<TARGET_FILE:test_basic>)
add_test(NAME preload_adaptive_demo
         COMMAND ${CMAKE_COMMAND} -E env
         LD_PRELOAD=$<TARGET_FILE:my_ptmalloc>
         MY_MALLOC_MODE=adaptive_demo
         $<TARGET_FILE:test_basic>)
