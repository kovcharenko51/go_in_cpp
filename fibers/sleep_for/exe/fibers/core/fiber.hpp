#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/coroutine/impl.hpp>

#include <context/stack.hpp>

#include <twist/stdlike/atomic.hpp>

namespace exe::fibers {

// Fiber = Stackful coroutine + Scheduler

class Fiber {
 public:
  Fiber(Routine routine, Scheduler& scheduler);
  ~Fiber();
  // ~ System calls

  void Schedule();

  void Yield();
  void SleepFor(Millis delay);

  void Suspend();
  void Resume();

  static Fiber& Self();

  Scheduler* GetScheduler();

 private:
  // Task
  void Step();
  void Dispatch();

 private:
  Scheduler* scheduler_;
  context::Stack stack_;
  coroutine::CoroutineImpl coroutine_;
};

}  // namespace exe::fibers
