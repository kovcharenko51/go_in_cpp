#pragma once

#include "atomic_stamped_ptr.hpp"

#include <twist/stdlike/atomic.hpp>

#include <optional>

// Treiber unbounded lock-free stack
// https://en.wikipedia.org/wiki/Treiber_stack

template <typename T>
class LockFreeStack {
  struct Node {
    T value;
    StampedPtr<Node> next;

    twist::stdlike::atomic<int> dec_count{0};

    explicit Node(T value) : value(std::move(value)), next({nullptr, 0}) {
    }
  };

 public:
  void Push(T value) {
    auto new_head = StampedPtr<Node>{new Node(std::move(value)), 0};
    while (!top_.CompareExchangeWeak(new_head->next, new_head)) {
    }
  }

  std::optional<T> TryPop() {
    StampedPtr<Node> old_head;
    bool has_changed = false;
    std::optional<T> result;
    do {
      old_head = top_.Load();
      while (!top_.CompareExchangeWeak(old_head, old_head.IncrementStamp())) {
      }
      old_head = old_head.IncrementStamp();

      if (old_head.raw_ptr == nullptr) {
        return std::nullopt;
      }

      auto head = old_head;
      has_changed = top_.CompareExchangeWeak(head, head->next);

      int delta = 1;
      if (has_changed) {
        result = std::move(old_head->value);
        delta -= old_head.stamp;
      }

      auto dec_count = old_head->dec_count.fetch_add(delta);
      dec_count += delta;
      if (dec_count == 0) {
        delete old_head.raw_ptr;
      }
    } while (!has_changed);

    return result;
  }

  ~LockFreeStack() {
    auto temp = top_.Load();
    while (temp.raw_ptr != nullptr) {
      auto to_delete = temp.raw_ptr;
      temp = temp->next;
      delete to_delete;
    }
  }

 private:
  AtomicStampedPtr<Node> top_{{nullptr, 0}};
};
