#pragma once

#include <wheels/intrusive/forward_list.hpp>

namespace exe::executors {

struct ITask {
  virtual ~ITask() = default;

  virtual void Run() noexcept = 0;
  virtual void Discard() noexcept = 0;
};

// Intrusive task
struct TaskBase : ITask, wheels::IntrusiveForwardListNode<TaskBase> {};

template <typename F>
struct Task : TaskBase {
  explicit Task(F f) : task_(std::move(f)) {
  }

  void Run() noexcept {
    try {
      task_();
    } catch (...) {
    }
    delete this;
  }

  void Discard() noexcept {
    delete this;
  }

 private:
  F task_;
};

}  // namespace exe::executors
