#include <exe/coroutine/impl.hpp>

#include <wheels/support/assert.hpp>
#include <wheels/support/compiler.hpp>

namespace exe::coroutine {

CoroutineImpl::CoroutineImpl(Routine routine, wheels::MutableMemView stack) {
  routine_ = std::move(routine);
  routine_context_.Setup(stack, this);
}

void CoroutineImpl::Run() {
  try {
    routine_();
  } catch (...) {
    exception_ptr_ = std::current_exception();
  }
  is_completed_ = true;
  routine_context_.ExitTo(context_);
}

void CoroutineImpl::Resume() {
  context_.SwitchTo(routine_context_);
  if (exception_ptr_ != nullptr) {
    std::rethrow_exception(exception_ptr_);
  }
}

void CoroutineImpl::Suspend() {
  routine_context_.SwitchTo(context_);
}

bool CoroutineImpl::IsCompleted() const {
  return is_completed_;
}

}  // namespace exe::coroutine
