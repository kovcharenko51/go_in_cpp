#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>

namespace solutions {

// CyclicBarrier allows a set of threads to all wait for each other
// to reach a common barrier point

// The barrier is called cyclic because
// it can be re-used after the waiting threads are released.

class CyclicBarrier {
 public:
  explicit CyclicBarrier(size_t thread_count)
      : thread_count_(thread_count), waiting_counts_{0, 0}, barrier_state_(0) {
  }

  // Blocks until all participants have invoked Arrive()
  void Arrive() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    int current_state = barrier_state_;
    waiting_counts_[current_state]++;
    while (waiting_counts_[current_state] != thread_count_) {
      has_waiting_.wait(lock);
    }
    if (barrier_state_ == current_state) {
      barrier_state_ = 1 - barrier_state_;
      waiting_counts_[barrier_state_] = 0;
      has_waiting_.notify_all();
    }
  }

 private:
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable has_waiting_;
  const size_t thread_count_;
  size_t waiting_counts_[2];
  int barrier_state_;
};

}  // namespace solutions
