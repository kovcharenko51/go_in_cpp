#pragma once

#include "twist/stdlike/mutex.hpp"
#include "twist/stdlike/condition_variable.hpp"

#include <wheels/intrusive/forward_list.hpp>
#include <exe/executors/task.hpp>

#include <optional>
#include <deque>

namespace exe::executors::tp::compute {
// Unbounded blocking multi-producers/multi-consumers queue

class UnboundedBlockingQueue {
 public:
  bool Put(wheels::IntrusiveForwardListNode<TaskBase>* value) {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    if (is_open_) {
      buffer_.PushBack(std::move(value));
      lock.unlock();
      not_empty_.notify_one();
      return true;
    }
    return false;
  }

  TaskBase* Take() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    while (buffer_.IsEmpty()) {
      if (!is_open_) {
        return nullptr;
      }
      not_empty_.wait(lock);
    }
    return std::move(buffer_.PopFront());
  }

  void Close() {
    CloseImpl(false);
  }

  void Cancel() {
    CloseImpl(true);
  }

 private:
  void CloseImpl(bool clear) {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    is_open_ = false;
    if (clear) {
      while (!buffer_.IsEmpty()) {
        auto task = buffer_.PopFront();
        task->Discard();
      }
    }
    lock.unlock();
    not_empty_.notify_all();
  }

 private:
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable not_empty_;
  wheels::IntrusiveForwardList<TaskBase> buffer_;
  bool is_open_ = true;
};

}  // namespace exe::executors::tp::compute
