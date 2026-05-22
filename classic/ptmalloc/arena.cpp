// Arena implementation.

#include "arena.h"

namespace my_ptmalloc {
namespace ptmalloc {

Arena::Arena() noexcept
    : top_(nullptr), last_remainder_(nullptr)
    , next_(nullptr), next_free_(nullptr)
    , attached_threads_(0), system_mem_(0), max_system_mem_(0)
    , is_main_(false)
{
    mutex_ = PTHREAD_MUTEX_INITIALIZER;
}

void Arena::init(bool is_main) noexcept {
    mutex_ = PTHREAD_MUTEX_INITIALIZER;
    top_ = nullptr;
    last_remainder_ = nullptr;
    next_ = nullptr;
    next_free_ = nullptr;
    attached_threads_ = is_main ? 1 : 0;
    system_mem_ = 0;
    max_system_mem_ = 0;
    is_main_ = is_main;
    bins_.init();
}

} // namespace ptmalloc
} // namespace my_ptmalloc
