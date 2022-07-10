#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/stacks.hpp>

#include <twist/util/thread_local.hpp>

#include <asio/steady_timer.hpp>
#include <asio/defer.hpp>
#include <asio/post.hpp>
#include <twist/util/spin_wait.hpp>

namespace exe::fibers {

static twist::util::ThreadLocalPtr<Fiber> running;

//////////////////////////////////////////////////////////////////////

Fiber::Fiber(Routine routine, Scheduler& scheduler)
    : scheduler_(&scheduler),
      stack_(AllocateStack()),
      coroutine_(std::move(routine), stack_.View()) {
}

Fiber::~Fiber() {
  ReleaseStack(std::move(stack_));
}

void Fiber::Schedule() {
  asio::post(*scheduler_, [this]() {
    this->Resume();
  });
}

void Fiber::Yield() {
  asio::defer(*scheduler_, [&]() {
    Resume();
  });
  Suspend();
}

void Fiber::Step() {
  running = this;
  coroutine_.Resume();
}

Fiber& Fiber::Self() {
  return *running;
}

void Fiber::Suspend() {
  coroutine_.Suspend();
}

void Fiber::Resume() {
  Step();
  Dispatch();
}

void Fiber::Dispatch() {
  if (coroutine_.IsCompleted()) {
    delete this;
  }
}

void Fiber::SleepFor(Millis delay) {
  asio::steady_timer timer(*scheduler_);

  timer.expires_after(delay);

  asio::defer(*scheduler_, [&]() {
    timer.async_wait([&](std::error_code) {
      Resume();
    });
  });
  Suspend();
}

Scheduler* Fiber::GetScheduler() {
  return scheduler_;
}

//////////////////////////////////////////////////////////////////////

// API Implementation

void Go(Scheduler& scheduler, Routine routine) {
  Fiber* fiber = new Fiber(std::move(routine), scheduler);
  fiber->Schedule();
}

void Go(Routine routine) {
  Fiber* fiber = new Fiber(std::move(routine), *Fiber::Self().GetScheduler());
  fiber->Schedule();
}

namespace self {

void Yield() {
  exe::fibers::running->Yield();
}

void SleepFor(Millis delay) {
  exe::fibers::running->SleepFor(delay);
}

}  // namespace self

}  // namespace exe::fibers
