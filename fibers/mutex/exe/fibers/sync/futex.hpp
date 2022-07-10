#pragma once

#include <exe/fibers/core/api.hpp>

#include <exe/fibers/core/fiber.hpp>
#include <exe/fibers/core/awaiter.hpp>
#include <exe/fibers/core/handle.hpp>

#include <exe/support/spinlock.hpp>

#include <twist/stdlike/atomic.hpp>

#include <wheels/intrusive/list.hpp>
#include <wheels/support/assert.hpp>

namespace exe::fibers {

template <typename T>
class FutexLike {
  class FutexNode : public IAwaiter,
                    public wheels::IntrusiveListNode<FutexNode> {
   public:
    explicit FutexNode(support::SpinLock& lock) : lock_(lock) {
    }

    void AwaitSuspend(FiberHandle handle) {
      handle_ = handle;
      lock_.unlock();
    }

    void Schedule() {
      handle_.Schedule();
    }

   private:
    FiberHandle handle_;
    support::SpinLock& lock_;
  };

 public:
  explicit FutexLike(twist::stdlike::atomic<T>& cell)
      : cell_(cell), spinlock_(support::SpinLock()) {
  }

  ~FutexLike() {
    assert(queue_.IsEmpty());
  }

  // Park current fiber if cell.load() == `old`
  void ParkIfEqual(T old) {
    spinlock_.lock();
    if (cell_.load() == old) {
      FutexNode node = FutexNode(spinlock_);
      queue_.PushBack(&node);
      self::Suspend(&node);
    } else {
      spinlock_.unlock();
    }
  }

  void WakeOne() {
    std::lock_guard<exe::support::SpinLock> lock(spinlock_);
    if (!queue_.IsEmpty()) {
      auto node = queue_.PopFront();
      node->Schedule();
    }
  }

  void WakeAll() {
    std::lock_guard<exe::support::SpinLock> lock(spinlock_);
    while (!queue_.IsEmpty()) {
      auto node = queue_.PopFront();
      node->Schedule();
    }
  }

 private:
  wheels::IntrusiveList<FutexNode> queue_;
  twist::stdlike::atomic<T>& cell_;
  support::SpinLock spinlock_;
};

}  // namespace exe::fibers
