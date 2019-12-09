#pragma once

#include <memory>
#include <assert.h>

namespace sb
{

struct ServiceState
{
    bool isReady = false;
};


template<class T>
class ServiceRef
{
public:
    ServiceRef(std::shared_ptr<T> service, ServiceState* state)
        : service_(service) {}

    T* operator->()
    {
        return get();
    }

    T * get()
    {
        assert (isReady());
        return service_.get();
    }

    bool isReady()
    {
        return !state_ || state_->isReady;
    }

    T& operator*()
    {
        return *service_;
    }

private:
    std::shared_ptr<T> service_;
    ServiceState* state_;
};

}