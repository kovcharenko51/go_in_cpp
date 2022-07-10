#pragma once

#include <exe/fibers/sync/futex.hpp>
#include <exe/fibers/sync/mutex.hpp>
#include <exe/fibers/sync/condvar.hpp>
#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

// https://gobyexample.com/waitgroups

class WaitGroup {
 public:
  void Add(size_t count) {
    std::lock_guard<Mutex> lock(mutex_);
    count_.fetch_add(count);
  }

  void Done() {
    doing_.fetch_add(1);
    std::unique_lock<Mutex> lock(mutex_);
    count_.fetch_sub(1);
    has_waiting_.NotifyAll();
    lock.unlock();
    doing_.fetch_sub(1);
  }

  void Wait() {
    std::unique_lock<Mutex> lock(mutex_);
    while (count_.load() > 0) {
      has_waiting_.Wait(lock);
    }
  }

  ~WaitGroup() {
    while (doing_.load() > 0) {
    }
  }

 private:
  Mutex mutex_;
  twist::stdlike::atomic<uint32_t> count_{0};
  twist::stdlike::atomic<uint32_t> doing_{0};
  CondVar has_waiting_;
};

}  // namespace exe::fibers
