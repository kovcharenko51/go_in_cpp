#pragma once

#include <twist/stdlike/mutex.hpp>
#include <twist/stdlike/condition_variable.hpp>

#include <optional>
#include <deque>

namespace tp {

// Unbounded blocking multi-producers/multi-consumers queue

template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T value) {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    if (is_open_) {
      buffer_.push_back(std::move(value));
      if (!buffer_.empty()) {
        not_empty_.notify_all();
      }
      return true;
    }
    return false;
  }

  std::optional<T> Take() {
    std::unique_lock<twist::stdlike::mutex> lock(mutex_);
    while (buffer_.empty()) {
      if (!is_open_) {
        return std::nullopt;
      }
      not_empty_.wait(lock);
    }
    T value = std::move(buffer_.front());
    buffer_.pop_front();
    return value;
  }

  void Close() {
    CloseImpl(false);
  }

  void Cancel() {
    CloseImpl(true);
  }

 private:
  void CloseImpl(bool clear) {
    std::lock_guard<twist::stdlike::mutex> lock(mutex_);
    is_open_ = false;
    if (clear) {
      while (!buffer_.empty()) {
        buffer_.pop_front();
      }
    }
    not_empty_.notify_all();
  }

 private:
  twist::stdlike::mutex mutex_;
  twist::stdlike::condition_variable not_empty_;
  std::deque<T> buffer_;
  bool is_open_ = true;
};

}  // namespace tp
