#pragma once

#include <exe/tasks/sync/one_shot_event.hpp>

#include <twist/stdlike/atomic.hpp>

namespace exe::tasks {

class WaitGroup {
 public:
  void Add(size_t count) {
    count_.fetch_add(count);
  }

  void Done() {
    uint32_t count = count_.fetch_sub(1);

    if (count == 1) {
      one_shot_event_.Fire();
    }
  }

  // Asynchronous
  auto Wait() {
    return one_shot_event_.Wait();
  }

 private:
  twist::stdlike::atomic<uint32_t> count_{0};
  OneShotEvent one_shot_event_;
};

}  // namespace exe::tasks
