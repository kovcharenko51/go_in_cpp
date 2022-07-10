#include <exe/executors/manual.hpp>

namespace exe::executors {

void ManualExecutor::Execute(TaskBase* task) {
  tasks_.PushBack(std::move(task));
}

// Run tasks

size_t ManualExecutor::RunAtMost(size_t limit) {
  for (size_t i = 0; i < limit; ++i) {
    if (tasks_.IsEmpty()) {
      return i;
    }
    auto task = std::move(tasks_.PopFront());
    task->Run();
  }
  return limit;
}

size_t ManualExecutor::Drain() {
  uint32_t count = 0;
  while (RunNext()) {
    count++;
  }
  return count;
}

}  // namespace exe::executors
