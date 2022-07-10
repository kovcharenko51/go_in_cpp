#pragma once

#include <exe/fibers/core/handle.hpp>
#include <exe/support/spinlock.hpp>

#include <wheels/intrusive/list.hpp>
#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

struct IAwaiter {
  virtual void AwaitSuspend(FiberHandle) = 0;

  virtual ~IAwaiter() = default;
};

class YieldAwaiter : public IAwaiter {
 public:
  void AwaitSuspend(FiberHandle handle) {
    handle.Schedule();
  }
};

}  // namespace exe::fibers
