#include <exe/fibers/core/stacks.hpp>

#include <optional>
#include <vector>
#include <mutex>

#include "exe/support/spinlock.hpp"

using context::Stack;

namespace exe::fibers {

//////////////////////////////////////////////////////////////////////

class StackAllocator {
 public:
  Stack Allocate() {
    std::lock_guard lock(spin_lock_);
    if (auto stack = TryTakeFromPool()) {
      return std::move(*stack);
    }
    return AllocateNewStack();
  }

  void Release(Stack stack) {
    std::lock_guard lock(spin_lock_);
    pool_.push_back(std::move(stack));
  }

 private:
  std::optional<Stack> TryTakeFromPool() {
    if (pool_.empty()) {
      return std::nullopt;
    }

    Stack stack = std::move(pool_.back());
    pool_.pop_back();
    return stack;
  }

  static Stack AllocateNewStack() {
    static const size_t kStackPages = 16;  // 16 * 4KB = 64KB
    return Stack::AllocatePages(kStackPages);
  }

 private:
  std::vector<Stack> pool_;
  SpinLock spin_lock_;
};

//////////////////////////////////////////////////////////////////////

StackAllocator allocator;

context::Stack AllocateStack() {
  return allocator.Allocate();
}

void ReleaseStack(context::Stack stack) {
  allocator.Release(std::move(stack));
}

}  // namespace exe::fibers
