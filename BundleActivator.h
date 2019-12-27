#pragma once

#include "micro/ActivatorsRegistry.h"
#include "micro/BundleInstaceImpl.h"

namespace sb {

template<class T>
struct BundleActivator
{
  using Bundle = T;
  using Exports = typename Bundle::Exports;
  using Externals = typename Bundle::Externals;
  static constexpr auto BundleId = Bundle::BundleId;
};
}
