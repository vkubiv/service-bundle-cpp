#pragma once

#include <assert.h>
#include <memory>

namespace sb {

struct ServiceState
{
  bool isReady = false;
};

template<class T>
class ServiceRef
{
public:
  ServiceRef()
    : state_(0)
  {}

  ServiceRef(std::shared_ptr<T> service, ServiceState* state)
    : service_(service)
    , state_(state)
  {}

  T* operator->() { return get(); }

  T* get()
  {
    assert(isReady());
    return service_.get();
  }

  T* getUnsafe() { return service_.get(); }

  bool isReady() { return !state_ || state_->isReady; }

  T& operator*()
  {
    assert(isReady());
    return *service_;
  }

private:
  std::shared_ptr<T> service_;
  ServiceState* state_;
};

} // namespace sb