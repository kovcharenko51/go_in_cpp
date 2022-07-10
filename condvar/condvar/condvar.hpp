#pragma once

#include <twist/stdlike/atomic.hpp>

#include <cstdint>

namespace stdlike {

class CondVar {
 public:
  // Mutex - BasicLockable
  // https://en.cppreference.com/w/cpp/named_req/BasicLockable
  template <class Mutex>
  void Wait(Mutex& mutex) {
    auto value = value_.load();
    mutex.unlock();
    value_.FutexWait(value);
    mutex.lock();
  }

  void NotifyOne() {
    value_.fetch_add(1);
    value_.FutexWakeOne();
  }

  void NotifyAll() {
    value_.fetch_add(1);
    value_.FutexWakeAll();
  }

 private:
  twist::stdlike::atomic<uint32_t> value_{0};
};

}  // namespace stdlike
