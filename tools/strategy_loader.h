#pragma once

#include "my_ptmalloc/strategy.h"

#include <cstdio>
#include <cstring>
#include <dlfcn.h>

namespace my_ptmalloc::tools {

struct LoadedStrategy {
    StrategyDescriptor desc{};
    void* handle = nullptr;
};

inline bool load_strategy(const char* spec, LoadedStrategy& out) {
    if (!spec) spec = "adaptive";
    // Classic ptmalloc reproduction (standalone)
    if (std::strcmp(spec, "ptmalloc") == 0) {
        out.desc = ptmalloc_classic_strategy_descriptor();
        return true;
    }
    // Classic tcmalloc reproduction (standalone)
    if (std::strcmp(spec, "tcmalloc") == 0 || std::strcmp(spec, "tcmalloc_like") == 0) {
        out.desc = tcmalloc_classic_strategy_descriptor();
        return true;
    }
    // Classic jemalloc reproduction (standalone)
    if (std::strcmp(spec, "jemalloc") == 0 || std::strcmp(spec, "jemalloc_like") == 0) {
        out.desc = jemalloc_classic_strategy_descriptor();
        return true;
    }
    // Classic mimalloc reproduction (standalone)
    if (std::strcmp(spec, "mimalloc") == 0 || std::strcmp(spec, "mimalloc_like") == 0) {
        out.desc = mimalloc_classic_strategy_descriptor();
        return true;
    }
    if (std::strcmp(spec, "adaptive") == 0) {
        out.desc = adaptive_strategy_descriptor();
        return true;
    }
    if (std::strcmp(spec, "libc") == 0 || std::strcmp(spec, "glibc") == 0) {
        out.desc = libc_strategy_descriptor();
        return true;
    }

    const char* path = spec;
    constexpr const char* prefix = "plugin:";
    if (std::strncmp(spec, prefix, std::strlen(prefix)) == 0) {
        path = spec + std::strlen(prefix);
    }

    void* handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        std::fprintf(stderr, "failed to load plugin '%s': %s\n", path, dlerror());
        return false;
    }

    auto* sym = dlsym(handle, "my_malloc_get_strategy");
    if (!sym) {
        std::fprintf(stderr, "plugin '%s' does not export my_malloc_get_strategy\n", path);
        dlclose(handle);
        return false;
    }

    auto entry = reinterpret_cast<StrategyEntryFn>(sym);
    out.desc = entry();
    out.handle = handle;
    if (out.desc.api_version != STRATEGY_API_VERSION) {
        std::fprintf(stderr, "plugin api mismatch: got %u expected %u\n",
                     out.desc.api_version, STRATEGY_API_VERSION);
        dlclose(handle);
        out.handle = nullptr;
        return false;
    }
    return true;
}

inline void unload_strategy(LoadedStrategy& s) {
    if (s.desc.vtable.shutdown) s.desc.vtable.shutdown();
    if (s.handle) dlclose(s.handle);
    s.handle = nullptr;
}

} // namespace my_ptmalloc::tools
