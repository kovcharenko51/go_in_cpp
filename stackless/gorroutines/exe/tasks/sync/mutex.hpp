#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

#include <experimental/coroutine>

// std::unique_lock
#include <mutex>
#include <cstdint>

namespace exe::tasks {

class Mutex {
  using UniqueLock = std::unique_lock<Mutex>;

  struct Locker;

  static uintptr_t const kUnlocked = 0;
  static uintptr_t const kLocked = 1;

  struct [[nodiscard]] Locker {
    Mutex& mutex_;
    Locker* next_{nullptr};
    std::experimental::coroutine_handle<> handle_;

    explicit Locker(Mutex& mutex) : mutex_(mutex) {
    }

    // Awaiter protocol

    // NOLINTNEXTLINE
    bool await_ready() {
      return mutex_.TryLock(this);
    }

    // NOLINTNEXTLINE
    bool await_suspend(std::experimental::coroutine_handle<> handle) {
      handle_ = handle;
      return !mutex_.TryLockOrEnqueue(this);
    }

    // NOLINTNEXTLINE
    UniqueLock await_resume() {
      return UniqueLock(mutex_);
    }
  };

 public:
  // Asynchronous
  auto ScopedLock() {
    return Locker{*this};
  }

  // For std::unique_lock
  // Do not use directly
  void unlock() {  // NOLINT
    Unlock();
  }

  void lock() {  // NOLINT
  }

 private:
  // Returns true if lock acquired

  bool TryLock(Locker* locker) {
    return head_.compare_exchange_strong(locker->next_,
                                         reinterpret_cast<Locker*>(kLocked));
  }

  bool TryLockOrEnqueue(Locker* locker) {
    while (true) {
      if (locker->next_ == reinterpret_cast<Locker*>(kUnlocked)) {
        if (head_.compare_exchange_weak(locker->next_,
                                        reinterpret_cast<Locker*>(kLocked))) {
          return true;
        }
      } else if (head_.compare_exchange_weak(locker->next_, locker)) {
        return false;
      }
    }
  }

  void Unlock() {
    auto tail = tail_;
    if (tail != nullptr) {
      tail_ = tail->next_;
      tail->handle_.resume();
    } else {
      auto state = reinterpret_cast<Locker*>(kLocked);
      if (head_.compare_exchange_strong(state,
                                        reinterpret_cast<Locker*>(kUnlocked))) {
        return;
      }

      auto to_reverse = head_.exchange(reinterpret_cast<Locker*>(kLocked));
      to_reverse = Reverse(to_reverse);
      tail_ = to_reverse;

      tail_ = to_reverse->next_;
      to_reverse->handle_.resume();
    }
  }

 private:
  Locker* Reverse(Locker* locker) {
    Locker* cur = locker;
    auto next = locker->next_;
    cur->next_ = nullptr;

    while (next != reinterpret_cast<Locker*>(kLocked)) {
      auto temp = next->next_;
      next->next_ = cur;

      cur = next;
      next = temp;
    }

    return cur;
  }

  twist::stdlike::atomic<Locker*> head_{nullptr};
  Locker* tail_{nullptr};
};

}  // namespace exe::tasks
