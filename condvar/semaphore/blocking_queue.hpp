#pragma once

#include "tagged_semaphore.hpp"

#include <deque>

namespace solutions {

// Bounded Blocking Multi-Producer/Multi-Consumer (MPMC) Queue

template <typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(size_t capacity)
      : free_semaphore_(capacity), count_semaphore_(0), guard_semaphore_(1) {
  }

  // Inserts the specified element into this queue,
  // waiting if necessary for space to become available.
  void Put(T value) {
    auto token = free_semaphore_.Acquire();
    guard_semaphore_.Acquire();
    buffer_.push_back(std::move(value));
    guard_semaphore_.Release();
    count_semaphore_.Release(std::move(token));
  }

  // Retrieves and removes the head of this queue,
  // waiting if necessary until an element becomes available
  T Take() {
    auto token = count_semaphore_.Acquire();
    guard_semaphore_.Acquire();
    T value = std::move(buffer_.front());
    buffer_.pop_front();
    guard_semaphore_.Release();
    free_semaphore_.Release(std::move(token));
    return value;
  }

 private:
  TaggedSemaphore<int> free_semaphore_;
  TaggedSemaphore<int> count_semaphore_;
  Semaphore guard_semaphore_;
  std::deque<T> buffer_;
};

}  // namespace solutions
