#pragma once

#include <twist/stdlike/atomic.hpp>
#include <twist/util/spin_wait.hpp>

namespace spinlocks {

/*  Scalable Queue SpinLock
 *
 *  Usage:
 *
 *  QueueSpinLock qspinlock;
 *  {
 *    QueueSpinLock::Guard guard(qspinlock);  // <-- Acquire
 *    // Critical section
 *  }  // <-- Release
 */

class QueueSpinLock {
 public:
  class Guard {
    friend class QueueSpinLock;

   public:
    explicit Guard(QueueSpinLock& spinlock) : spinlock_(spinlock) {
      spinlock_.Acquire(this);
    }

    ~Guard() {
      spinlock_.Release(this);
    }

   private:
    QueueSpinLock& spinlock_;
    twist::stdlike::atomic<Guard*> next_{nullptr};
    twist::stdlike::atomic<bool> owner_{false};
  };

 private:
  void Acquire(Guard* guard) {
    auto prev_tail = tail_.exchange(guard);
    if (prev_tail != nullptr) {
      prev_tail->next_.store(guard);
    } else {
      guard->owner_.store(true);
    }
    WaitAcquire(guard);
  }

  void Release(Guard* owner) {
    Guard* owner_copy = owner;
    if (!tail_.compare_exchange_strong(owner_copy, nullptr)) {
      WaitRelease(owner);
      owner->next_.load()->owner_.store(true);
    }
  }

  void WaitRelease(Guard* owner) {
    twist::util::SpinWait spin_wait;
    while (owner->next_.load() == nullptr) {
      spin_wait.Spin();
    }
  }

  void WaitAcquire(Guard* guard) {
    twist::util::SpinWait spin_wait;
    while (!guard->owner_.load()) {
      spin_wait.Spin();
    }
  }

 private:
  twist::stdlike::atomic<Guard*> tail_{nullptr};
};

}  // namespace spinlocks
