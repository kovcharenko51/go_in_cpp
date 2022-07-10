#pragma once

#include <twist/stdlike/atomic.hpp>
#include <wheels/intrusive/forward_list.hpp>
#include <exe/executors/task.hpp>

#include <optional>
#include <deque>

#include <twist/util/spin_wait.hpp>

namespace exe::executors {

class LockFreeStack {
  using Node = wheels::IntrusiveForwardListNode<TaskBase>;

 public:
  void Push(Node* node) {
    node->next_ = top_.load();
    twist::util::SpinWait spin_wait;
    while (!top_.compare_exchange_weak(node->next_, node)) {
      spin_wait.Spin();
    }
  }

  std::deque<TaskBase*> PopAll() {
    Node* temp = top_.exchange(nullptr);
    std::deque<TaskBase*> values;
    while (temp != nullptr) {
      values.push_front(std::move(temp->AsItem()));
      temp = temp->next_;
    }
    return values;
  }

  bool IsEmpty() {
    return top_.load() == nullptr;
  }

  twist::stdlike::atomic<Node*> top_{nullptr};
};

};  // namespace exe::executors