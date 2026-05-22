cmake_minimum_required(VERSION 3.14)
project(my_ptmalloc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ─── Classic allocator reproductions (independent, self-contained) ───
add_subdirectory(classic/ptmalloc)
add_subdirectory(classic/tcmalloc)
add_subdirectory(classic/jemalloc)
add_subdirectory(classic/mimalloc)

# ─── Adaptive allocator (shared library for LD_PRELOAD) ───
file(GLOB ADAPTIVE_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(my_ptmalloc SHARED ${ADAPTIVE_SOURCES})
target_include_directories(my_ptmalloc PUBLIC include)
target_link_libraries(my_ptmalloc PRIVATE pthread dl)
target_compile_options(my_ptmalloc PRIVATE -O2 -march=native -Wall -Wextra -Wno-unused-parameter)

# Static library for linking into tools
add_library(my_ptmalloc_static STATIC ${ADAPTIVE_SOURCES})
target_include_directories(my_ptmalloc_static PUBLIC include)
target_link_libraries(my_ptmalloc_static PRIVATE pthread dl)
target_compile_options(my_ptmalloc_static PRIVATE -O2 -march=native -Wall -Wextra -Wno-unused-parameter)

# ─── Tests ───
enable_testing()

# Adaptive mode tests
add_executable(test_adaptive_modes test/test_adaptive_modes.cpp)
target_link_libraries(test_adaptive_modes PRIVATE my_ptmalloc_static pthread)
target_compile_options(test_adaptive_modes PRIVATE -O2 -march=native)
foreach(mode balanced throughput_cache compact_rss large_object)
  add_test(NAME adaptive_mode_${mode}
           COMMAND ${CMAKE_COMMAND} -E env
           MY_MALLOC_ADAPTIVE_MODE=${mode}
           MY_MALLOC_ADAPTIVE_MODE_SELECTOR=fixed
           $<TARGET_FILE:test_adaptive_modes> ${mode})
endforeach()
add_test(NAME adaptive_mode_hardened_debug
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=hardened_debug
         MY_MALLOC_ADAPTIVE_DEBUG_MODE=1
         $<TARGET_FILE:test_adaptive_modes> hardened_debug)
add_test(NAME adaptive_mode_soft_switch
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=balanced
         MY_MALLOC_ADAPTIVE_MODE_SELECTOR=manual
         $<TARGET_FILE:test_adaptive_modes> soft_switch)
add_test(NAME adaptive_throughput_tcache
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=throughput_cache
         MY_MALLOC_ADAPTIVE_MODE_SELECTOR=manual
         $<TARGET_FILE:test_adaptive_modes> throughput_tcache)
add_test(NAME adaptive_cross_thread_remote_queue
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=cross_thread
         MY_MALLOC_ADAPTIVE_MODE_SELECTOR=manual
         $<TARGET_FILE:test_adaptive_modes> remote_queue)
add_test(NAME adaptive_deterministic_latency_cache
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=deterministic_latency
         MY_MALLOC_ADAPTIVE_MODE_SELECTOR=manual
         $<TARGET_FILE:test_adaptive_modes> deterministic_cache)
add_test(NAME adaptive_fragmentation_stable_storage
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=fragmentation_stable
         MY_MALLOC_ADAPTIVE_MODE_SELECTOR=manual
         $<TARGET_FILE:test_adaptive_modes> fragmentation_storage)
add_test(NAME adaptive_compact_precise_reclaim
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=compact_rss
         MY_MALLOC_ADAPTIVE_MODE_SELECTOR=manual
         $<TARGET_FILE:test_adaptive_modes> compact_reclaim)
add_test(NAME adaptive_hardened_redzone
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=hardened_debug
         MY_MALLOC_ADAPTIVE_DEBUG_MODE=1
         $<TARGET_FILE:test_adaptive_modes> hardened_redzone)
add_test(NAME adaptive_window_delta_features
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=balanced
         MY_MALLOC_ADAPTIVE_MODE_SELECTOR=manual
         $<TARGET_FILE:test_adaptive_modes> window_delta)
add_test(NAME adaptive_selector_remote_free
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=auto
         MY_MALLOC_ADAPTIVE_MODE_SELECTOR=rule
         MY_MALLOC_ADAPTIVE_MODE_WINDOW=64
         MY_MALLOC_ADAPTIVE_MODE_COOLDOWN=0
         $<TARGET_FILE:test_adaptive_modes> remote_selector)
add_test(NAME adaptive_selector_large_object
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=auto
         MY_MALLOC_ADAPTIVE_MODE_SELECTOR=rule
         MY_MALLOC_ADAPTIVE_MODE_WINDOW=32
         MY_MALLOC_ADAPTIVE_MODE_COOLDOWN=0
         $<TARGET_FILE:test_adaptive_modes> large_selector)

# ─── Tools ───

# Strategy validation (tests all allocators through unified interface)
add_executable(allocator_validate tools/allocator_validate.cpp)
target_include_directories(allocator_validate PRIVATE include tools classic/common)
target_link_libraries(allocator_validate PRIVATE
    my_ptmalloc_static
    ptmalloc_classic tcmalloc_classic jemalloc_classic mimalloc_classic
    pthread dl)
target_compile_options(allocator_validate PRIVATE -O2 -march=native)

foreach(strategy ptmalloc tcmalloc jemalloc mimalloc adaptive libc)
  add_test(NAME validate_${strategy}
           COMMAND allocator_validate --strategy ${strategy})
endforeach()

# Benchmark runner
add_executable(bench_runner
    tools/bench_runner.cpp
    tools/telemetry_server.cpp)
target_include_directories(bench_runner PRIVATE include tools classic/common)
target_link_libraries(bench_runner PRIVATE
    my_ptmalloc_static
    ptmalloc_classic tcmalloc_classic jemalloc_classic mimalloc_classic
    pthread dl)
target_compile_options(bench_runner PRIVATE -O2 -march=native)
target_compile_definitions(bench_runner PRIVATE
    WEB_VIEWER_DIR="${CMAKE_SOURCE_DIR}/tools/web_viewer")

add_test(NAME generated_workload_adaptive
         COMMAND bench_runner --strategy adaptive --bench generated_workload
                 --workload-template adaptive_mix --iters 5000 --slots 256 --threads 2 --json)
add_test(NAME generated_workload_ptmalloc
         COMMAND bench_runner --strategy ptmalloc --bench generated_workload
                 --workload-template adaptive_mix --iters 5000 --slots 256 --threads 2 --json)
add_test(NAME generated_workload_adaptive_model_selector
         COMMAND ${CMAKE_COMMAND} -E env
         MY_MALLOC_ADAPTIVE_MODE=auto
         MY_MALLOC_ADAPTIVE_MODE_SELECTOR=model
         MY_MALLOC_ADAPTIVE_MODE_WINDOW=32
         MY_MALLOC_ADAPTIVE_MODE_COOLDOWN=0
         $<TARGET_FILE:bench_runner> --strategy adaptive --bench generated_workload
                 --workload-config ${CMAKE_SOURCE_DIR}/workloads/smoke_large_to_small.json
                 --workload-realtime --phase-ms 50 --phase-repeat 1
                 --slots 128 --threads 2 --json)

# LD_PRELOAD smoke test (adaptive)
add_test(NAME preload_adaptive
         COMMAND ${CMAKE_COMMAND} -E env
         LD_PRELOAD=$<TARGET_FILE:my_ptmalloc>
         ${CMAKE_COMMAND} -E echo "preload_ok")

# Example plugin
add_library(example_counting_strategy MODULE plugins/counting_malloc_strategy.cpp)
target_include_directories(example_counting_strategy PRIVATE include)
target_compile_options(example_counting_strategy PRIVATE -O2 -march=native -Wall -Wextra)
set_target_properties(example_counting_strategy PROPERTIES
    PREFIX ""
    OUTPUT_NAME "libcounting_malloc_strategy")
