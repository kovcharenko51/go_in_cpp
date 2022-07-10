#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

// std::lock_guard, std::unique_lock
#include <mutex>
#include <cstdint>

namespace solutions {

// A Counting semaphore

// Semaphores are often used to restrict the number of threads
// than can access some (physical or logical) resource

class Semaphore {
 public:
  // Creates a Semaphore with the given number of permits
  explicit Semaphore(size_t token_count) : token_counter_(token_count) {
  }

  // Acquires a permit from this semaphore,
  // blocking until one is available
  void Acquire() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    while (token_counter_ == 0) {
      has_tokens_.wait(lock);
    }
    token_counter_--;
  }

  // Releases a permit, returning it to the semaphore
  void Release() {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    token_counter_++;
    has_tokens_.notify_all();
  }

 private:
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable has_tokens_;
  size_t token_counter_;
};

}  // namespace solutions
