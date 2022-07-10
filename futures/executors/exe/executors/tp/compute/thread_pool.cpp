#include <exe/executors/tp/compute/thread_pool.hpp>

#include "twist/util/thread_local.hpp"

namespace exe::executors::tp::compute {
////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocalPtr<ThreadPool> pool;

////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(size_t workers_count) {
  StartWorkerThreads(workers_count);
}

ThreadPool::~ThreadPool() {
}

void ThreadPool::Execute(TaskBase* task) {
  wait_group_.Add(1);
  if (!tasks_.Put(task)) {
    task->Discard();
  }
}

void ThreadPool::WaitIdle() {
  wait_group_.Wait();
}

void ThreadPool::Stop() {
  tasks_.Cancel();
  for (auto& worker : workers_) {
    worker.join();
  }
}

ThreadPool* ThreadPool::Current() {
  return pool;
}

void ThreadPool::StartWorkerThreads(size_t workers_count) {
  for (size_t i = 0; i < workers_count; ++i) {
    workers_.emplace_back([this]() {
      WorkerRoutine();
    });
  }
}

void ThreadPool::WorkerRoutine() {
  pool = this;
  while (auto task = tasks_.Take()) {
    try {
      task->Run();
    } catch (...) {
    }
    wait_group_.Done();
  }
}

}  // namespace exe::executors::tp::compute
