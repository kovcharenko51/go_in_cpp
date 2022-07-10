#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <mutex>

namespace exe::executors::tp::compute {

class WaitGroup {
 public:
  void Add(size_t count) {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    count_ += count;
  }

  void Done() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    if (--count_ == 0) {
      lock.unlock();
      has_waiting_.notify_all();
    }
  }

  void Wait() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    while (count_ > 0) {
      has_waiting_.wait(lock);
    }
  }

 private:
  twist::stdlike::mutex mutex_;
  uint32_t count_ = 0;
  twist::stdlike::condition_variable has_waiting_;
};

}  // namespace exe::executors::tp::compute
