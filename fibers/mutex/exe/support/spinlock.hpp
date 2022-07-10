#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

namespace exe::support {

// Test-and-TAS spinlock

class SpinLock {
 public:
  SpinLock() : locked_(false) {
  }

  void Lock() {
    while (locked_.exchange(true, std::memory_order_acquire)) {
      twist::util::SpinWait spin_wait;
      while (locked_.load(std::memory_order::relaxed)) {
        spin_wait.Spin();
      }
    }
  }

  void Unlock() {
    locked_.store(false, std::memory_order_release);
  }

  // BasicLockable

  void lock() {  // NOLINT
    Lock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  twist::stdlike::atomic<bool> locked_;
};

}  // namespace exe::support
