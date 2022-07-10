#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

namespace exe::fibers {

static twist::util::ThreadLocalPtr<Fiber> running;

//////////////////////////////////////////////////////////////////////

Fiber::Fiber(Routine routine, Scheduler* scheduler = Scheduler::Current())
    : scheduler_(scheduler),
      stack_(AllocateStack()),
      coroutine_(std::move(routine), stack_.View()) {
}

Fiber::~Fiber() {
  ReleaseStack(std::move(stack_));
}

void Fiber::Schedule() {
  scheduler_->Submit([this]() {
    this->Step();
  });
}

void Fiber::Yield() {
  coroutine_.Suspend();
}

void Fiber::Step() {
  running = this;
  coroutine_.Resume();
  if (coroutine_.IsCompleted()) {
    delete this;
  } else {
    Schedule();
  }
  running = nullptr;
}

Fiber& Fiber::Self() {
  return *running;
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& scheduler, Routine routine) {
  Fiber* fiber = new Fiber(std::move(routine), &scheduler);
  fiber->Schedule();
}

void Go(Routine routine) {
  Fiber* fiber = new Fiber(std::move(routine));
  fiber->Schedule();
}

namespace self {

void Yield() {
  exe::fibers::running->Yield();
}

}  // namespace self

}  // namespace exe::fibers
