#pragma once

#include <exe/fibers/core/api.hpp>
#include <exe/fibers/core/handle.hpp>
#include <exe/coroutine/impl.hpp>
#include <exe/fibers/core/awaiter.hpp>
#include <context/stack.hpp>

namespace exe::fibers {

// Fiber = Stackful coroutine + Scheduler (Thread pool)

class Fiber {
 public:
  Fiber(Routine routine, Scheduler* scheduler);

  ~Fiber();
  // ~ System calls

  void Schedule();

  void Suspend(IAwaiter*);
  void Resume();

  static Fiber& Self();
  static FiberHandle GetHandle();

 private:
  // Task
  void Step();

 private:
  Scheduler* scheduler_;
  context::Stack stack_;
  coroutine::CoroutineImpl coroutine_;
  IAwaiter* awaiter_;
};

}  // namespace exe::fibers
