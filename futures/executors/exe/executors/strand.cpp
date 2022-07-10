#include <exe/executors/strand.hpp>
#include "execute.hpp"

namespace exe::executors {

Strand::Strand(IExecutor& underlying) : executor_(underlying) {
}

void Strand::Execute(TaskBase* task) {
  stack_.Push(std::move(task));
  if (!scheduled_.exchange(true)) {
    Schedule();
  }
}

void Strand::Schedule() {
  exe::executors::Execute(executor_, [&]() {
    auto tasks = stack_.PopAll();
    while (!tasks.empty()) {
      auto task = tasks.front();
      tasks.pop_front();
      task->Run();
    }
    scheduled_.store(false);
    if ((!stack_.IsEmpty()) && (!scheduled_.exchange(true))) {
      Schedule();
    }
  });
}

}  // namespace exe::executors
