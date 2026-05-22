#include "strategy_loader.h"

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

using my_ptmalloc::tools::LoadedStrategy;
using my_ptmalloc::tools::load_strategy;
using my_ptmalloc::tools::unload_strategy;

static bool check(bool cond, const char* msg) {
    if (!cond) std::fprintf(stderr, "FAIL: %s\n", msg);
    return cond;
}

static bool validate_basic(const my_ptmalloc::StrategyDescriptor& s) {
    void* p = s.vtable.allocate(64);
    if (!check(p != nullptr, "malloc 64 returned null")) return false;
    std::memset(p, 0xAB, 64);
    s.vtable.deallocate(p);

    void* z = s.vtable.allocate(0);
    if (!check(z != nullptr, "malloc 0 returned null")) return false;
    s.vtable.deallocate(z);
    return true;
}

static bool validate_realloc(const my_ptmalloc::StrategyDescriptor& s) {
    char* p = static_cast<char*>(s.vtable.allocate(32));
    if (!check(p != nullptr, "realloc initial alloc failed")) return false;
    for (int i = 0; i < 32; ++i) p[i] = static_cast<char>(i);

    char* q = static_cast<char*>(s.vtable.reallocate(p, 256));
    if (!check(q != nullptr, "realloc grow failed")) return false;
    for (int i = 0; i < 32; ++i) {
        if (!check(q[i] == static_cast<char>(i), "realloc did not preserve data")) {
            s.vtable.deallocate(q);
            return false;
        }
    }

    char* r = static_cast<char*>(s.vtable.reallocate(q, 16));
    if (!check(r != nullptr, "realloc shrink failed")) return false;
    for (int i = 0; i < 16; ++i) {
        if (!check(r[i] == static_cast<char>(i), "realloc shrink did not preserve data")) {
            s.vtable.deallocate(r);
            return false;
        }
    }
    s.vtable.deallocate(r);
    return true;
}

static bool validate_usable_size(const my_ptmalloc::StrategyDescriptor& s) {
    if (!s.vtable.usable_size) return true;
    void* p = s.vtable.allocate(123);
    if (!check(p != nullptr, "usable_size allocation failed")) return false;
    size_t usable = s.vtable.usable_size(p);
    bool ok = check(usable == 0 || usable >= 123, "usable_size smaller than request");
    s.vtable.deallocate(p);
    return ok;
}

static bool validate_random(const my_ptmalloc::StrategyDescriptor& s) {
    constexpr int slots = 512;
    std::vector<void*> ptrs(slots, nullptr);
    std::vector<size_t> sizes(slots, 0);
    std::mt19937 rng(123);
    std::uniform_int_distribution<size_t> size_dist(1, 4096);
    std::uniform_int_distribution<int> action_dist(0, 3);

    for (int i = 0; i < 50000; ++i) {
        int idx = static_cast<int>(rng() % slots);
        int action = action_dist(rng);
        if (!ptrs[idx] || action <= 1) {
            if (ptrs[idx]) s.vtable.deallocate(ptrs[idx]);
            sizes[idx] = size_dist(rng);
            ptrs[idx] = s.vtable.allocate(sizes[idx]);
            if (ptrs[idx]) std::memset(ptrs[idx], idx & 0xff, sizes[idx]);
        } else if (action == 2) {
            size_t new_size = size_dist(rng);
            void* p = s.vtable.reallocate(ptrs[idx], new_size);
            if (!check(p != nullptr, "random realloc failed")) return false;
            ptrs[idx] = p;
            sizes[idx] = new_size;
        } else {
            s.vtable.deallocate(ptrs[idx]);
            ptrs[idx] = nullptr;
            sizes[idx] = 0;
        }
    }

    for (void* p : ptrs) {
        if (p) s.vtable.deallocate(p);
    }
    return true;
}

int main(int argc, char** argv) {
    const char* strategy = "adaptive";
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--strategy") == 0) strategy = argv[++i];
    }

    LoadedStrategy loaded;
    if (!load_strategy(strategy, loaded)) return 2;
    if (loaded.desc.vtable.init) loaded.desc.vtable.init();

    bool ok = validate_basic(loaded.desc) &&
              validate_realloc(loaded.desc) &&
              validate_usable_size(loaded.desc) &&
              validate_random(loaded.desc);

    std::printf("%s validation: %s\n", loaded.desc.name, ok ? "PASS" : "FAIL");
    unload_strategy(loaded);
    return ok ? 0 : 1;
}
