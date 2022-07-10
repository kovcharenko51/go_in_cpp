#pragma once

#include <twist/stdlike/atomic.hpp>

#include <experimental/coroutine>

namespace exe::tasks {

class OneShotEvent {
  struct OneShotAwaiter {
    explicit OneShotAwaiter(OneShotEvent* one_shot)
        : one_shot_event_(one_shot){};

    // NOLINTNEXTLINE
    bool await_ready() {
      return one_shot_event_->CheckFire(this);
    }

    // NOLINTNEXTLINE
    bool await_suspend(std::experimental::coroutine_handle<> handle) {
      handle_ = handle;
      return !(one_shot_event_->Enqueue(this));
    }

    // NOLINTNEXTLINE
    void await_resume() {
    }

    std::experimental::coroutine_handle<> handle_;
    OneShotAwaiter* next_{nullptr};
    OneShotEvent* one_shot_event_;
  };

 public:
  // Asynchronous
  auto Wait() {
    return OneShotAwaiter(this);
  }

  // One-shot
  void Fire() {
    auto awaiter = tail_.exchange(reinterpret_cast<OneShotAwaiter*>(kAwaiter));

    while (awaiter != nullptr) {
      auto next = awaiter->next_;
      reinterpret_cast<OneShotAwaiter*>(awaiter)->handle_.resume();
      awaiter = next;
    }
  }

 private:
  bool CheckFire(OneShotAwaiter* awaiter) {
    awaiter->next_ = tail_.load();
    return (awaiter->next_ == reinterpret_cast<OneShotAwaiter*>(kAwaiter));
  }

  bool Enqueue(OneShotAwaiter* current) {
    while (!tail_.compare_exchange_weak(current->next_, current)) {
      if (current->next_ == reinterpret_cast<OneShotAwaiter*>(kAwaiter)) {
        return true;
      }
    }
    return false;
  }

  static uintptr_t const kAwaiter = 1;
  twist::stdlike::atomic<OneShotAwaiter*> tail_{nullptr};
};

}  // namespace exe::tasks
