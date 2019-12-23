#pragma once

#include <memory>

#include "TypeList.h"
#include "meta/TypeListUtils.h"
#include "meta/constructor.h"

namespace sb {
class TypeAsync;
class TypeSync;
class BundlesRealm;

enum PType
{
  Singletone,
  New
};

template<class I, class Imp = I>
struct Provider
{
  using Interface = I;
  using Implementation = Imp;

  template<class InjectionContext>
  ServiceRef<Interface> get(InjectionContext& injectionContext)
  {
    if (!instance_)
      instance_ = meta::ConstructType<Implementation>(injectionContext);

    return ServiceRef<Interface>(instance_, &serviceState_);
  }

  template<class InjectionContext>
  void beforeBundleActivate(InjectionContext& injectionContext)
  {
    get(injectionContext);
  }

  void afterBundleActivate() { serviceState_.isReady = true; }

private:
  std::shared_ptr<Interface> instance_;
  ServiceState serviceState_;
};

template<class Interface>
class ProvideOnActivate
{};
} // namespace sb

class ServiceBundle
{};
