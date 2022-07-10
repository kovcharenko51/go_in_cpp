#pragma once

#include <exe/fibers/sync/mutex.hpp>
#include <exe/fibers/sync/futex.hpp>

#include <twist/stdlike/atomic.hpp>

// std::unique_lock
#include <mutex>

namespace exe::fibers {

class CondVar {
  using Lock = std::unique_lock<Mutex>;

 public:
  CondVar() : value_(0), futex_(FutexLike<uint32_t>(value_)) {
  }

  void Wait(Lock& lock) {
    auto value = value_.load();
    lock.unlock();
    futex_.ParkIfEqual(value);
    lock.lock();
  }

  void NotifyOne() {
    value_.fetch_add(1);
    futex_.WakeOne();
  }

  void NotifyAll() {
    value_.fetch_add(1);
    futex_.WakeAll();
  }

 private:
  twist::stdlike::atomic<uint32_t> value_{0};

  FutexLike<uint32_t> futex_;
};

}  // namespace exe::fibers
