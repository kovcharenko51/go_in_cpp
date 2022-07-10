#pragma once

#include <exe/executors/executor.hpp>
#include <exe/executors/execute.hpp>
#include <exe/executors/task.hpp>

#include <experimental/coroutine>

namespace exe::tasks {

namespace detail {

struct TeleportAwaiter : executors::TaskBase {
  explicit TeleportAwaiter(executors::IExecutor& target) : target_(target) {
  }

  // NOLINTNEXTLINE
  bool await_ready() noexcept {
    return false;
  }

  // NOLINTNEXTLINE
  void await_suspend(std::experimental::coroutine_handle<> handle) {
    handle_ = handle;
    target_.Execute(this);
  }

  void Run() noexcept {
    handle_.resume();
  }

  void Discard() noexcept {
    handle_.destroy();
  }

  // NOLINTNEXTLINE
  void await_resume() {
  }

  executors::IExecutor& target_;
  std::experimental::coroutine_handle<> handle_;
};

}  // namespace detail

// Reschedule current coroutine to executor `target`
inline auto TeleportTo(executors::IExecutor& target) {
  return detail::TeleportAwaiter(target);
}

}  // namespace exe::tasks
