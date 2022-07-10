#pragma once

#include "twist/stdlike/atomic.hpp"
#include "twist/util/spin_wait.hpp"

namespace exe::fibers {

// Test-and-TAS spinlock

class SpinLock {
 public:
  SpinLock() : locked_(0) {
  }

  void Lock() {
    while (locked_.exchange(1, std::memory_order_acquire) == 1) {
      twist::util::SpinWait spin_wait;
      while (locked_.load(std::memory_order::relaxed) == 1) {
        spin_wait.Spin();
      }
    }
  }

  void Unlock() {
    locked_.store(0, std::memory_order_release);
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::stdlike::atomic<uint32_t> locked_;
};

}  // namespace exe::fibers
