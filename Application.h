#pragma once

#include "BundleLoader.h"

namespace sb {
template<class T>
class Application
{
public:
  AsyncActivateResult setupBundles()
  {
    createBundles(typename T::Bundles());
    loader_.setupBundles();
    return static_cast<T*>(this)->activate(loader_);
  }

private:
  template<class Bundle>
  int createBundle()
  {
    loader_.createBundle<Bundle>();
    return 0;
  }

  template<class... TT>
  void createBundles(TypeList<TT...> exports)
  {
    int _[] = { createBundle<TT>()... };
  }

  BundlesLoader loader_;
};
} // namespace sb