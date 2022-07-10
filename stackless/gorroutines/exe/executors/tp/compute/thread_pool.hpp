#pragma once

#include "exe/executors/executor.hpp"

#include "blocking_queue.hpp"

#include "twist/stdlike/thread.hpp"
#include "twist/stdlike/atomic.hpp"

#include "wait_group.hpp"

#include <vector>

namespace exe::executors::tp::compute {

// Fixed-size pool of worker threads

class ThreadPool : public IExecutor {
 public:
  explicit ThreadPool(size_t workers);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // IExecutor
  // Schedules task for execution in one of the worker threads
  void Execute(TaskBase* task);

  // Waits until outstanding work count has reached zero
  void WaitIdle();

  // Stops the worker threads as soon as possible
  // Pending tasks will be discarded
  void Stop();

  // Locates current thread pool from worker thread
  static ThreadPool* Current();

 private:
  void StartWorkerThreads(size_t);
  void WorkerRoutine();

  std::vector<twist::stdlike::thread> workers_;
  compute::UnboundedBlockingQueue tasks_;

  WaitGroup wait_group_;
};

}  // namespace exe::executors::tp::compute
