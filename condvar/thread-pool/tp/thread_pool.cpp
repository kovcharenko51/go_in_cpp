#include <tp/thread_pool.hpp>

#include <twist/util/thread_local.hpp>

namespace tp {

////////////////////////////////////////////////////////////////////////////////

static twist::util::ThreadLocalPtr<ThreadPool> pool;

////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(size_t workers_count) {
  StartWorkerThreads(workers_count);
}

ThreadPool::~ThreadPool() {
}

void ThreadPool::Submit(Task task) {
  wait_group_.Add(1);
  tasks_.Put(std::move(task));
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
      (*task)();
    } catch (...) {
    }
    wait_group_.Done();
  }
}

}  // namespace tp
