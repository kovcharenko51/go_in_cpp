#pragma once

#include <exe/executors/executor.hpp>

#include <twist/stdlike/atomic.hpp>
#include <twist/stdlike/mutex.hpp>

#include "lock_free_stack.hpp"

namespace exe::executors {

// Strand (serial executor, asynchronous mutex)
// Executes (via underlying executor) tasks
// non-concurrently and in FIFO order

class Strand : public IExecutor {
 public:
  explicit Strand(IExecutor& underlying);

  // IExecutor
  void Execute(TaskBase* task) override;

 private:
  void Schedule();

 private:
  IExecutor& executor_;
  LockFreeStack stack_;
  twist::stdlike::atomic<bool> scheduled_{false};
};

}  // namespace exe::executors
