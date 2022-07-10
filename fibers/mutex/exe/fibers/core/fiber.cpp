#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

static twist::util::ThreadLocalPtr<exe::tp::ThreadPool> pool;

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

void Fiber::Step() {
  running = this;
  Resume();
  if (coroutine_.IsCompleted()) {
    delete this;
  } else {
    awaiter_->AwaitSuspend(GetHandle());
  }
  running = nullptr;
}

Fiber& Fiber::Self() {
  return *running;
}

FiberHandle Fiber::GetHandle() {
  return FiberHandle(running);
}

void Fiber::Suspend(IAwaiter* awaiter) {
  awaiter_ = awaiter;
  coroutine_.Suspend();
}

void Fiber::Resume() {
  coroutine_.Resume();
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
  YieldAwaiter awaiter = YieldAwaiter();
  Suspend(&awaiter);
}

void Suspend(IAwaiter* awaiter) {
  running->Suspend(awaiter);
}

}  // namespace self

}  // namespace exe::fibers
