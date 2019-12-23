#pragma once

#include "ActivatorsRegistry.h"
#include "BundleInstaceImpl.h"
#include "ServiceBundle.h"
#include <map>
#include <memory>

namespace sb {

class BundlesLoader
{
public:
  BundlesLoader()
    : bundlesRealm_(std::make_shared<BundlesRealm>())
  {}

  template<class Bundle>
  void createBundle()
  {
    if (bundlesRealm_->hasBundle(Bundle::BundleId))
      throw std::logic_error(std::string("Bundle creating multiple times in "
                                         "the same loader. Bundle id: ") +
                             Bundle::BundleId);

    auto bundleFactory =
      ActivatorsRegistry::Instance().getFactory(Bundle::BundleId);
    std::shared_ptr<IBundleInstaceImpl> bundleImpl = bundleFactory();

    bundlesRealm_->addBundle(Bundle::BundleId, bundleImpl);
    bundles_.push_back(bundleImpl);
  }

  void setupBundles()
  {
    for (auto& bundle : bundles_) {
      bundle->linkReferences(*bundlesRealm_);
    }

    for (auto& bundle : bundles_) {
      bundle->createServices();
    }
  }

  template<class Bundle, class... Externals>
  std::future<BundleRef<Bundle>> activate(Externals&&... externals)
  {
    auto bundle = bundlesRealm_->getBundle(Bundle::BundleId);
    if (!bundle)
      throw std::logic_error(
        std::string("Bundle should be created before activation. Bundle id: ") +
        Bundle::BundleId);

    int _[] = {
      0, setExternal<Bundle, Externals>(*bundle, std::move(externals))...
    };

    return std::async(std::launch::deferred, [bundle]() {
      bundle->activate().get();

      return BundleRef<Bundle>(bundle.get());
    });
  }

private:
  template<class Bundle, class External>
  int setExternal(IBundleInstaceImpl& bundle, External&& externals)
  {
    using Externals = typename Bundle::Externals;

    constexpr int externalsIndex = meta::TypeIndex<External, Externals>::value;
    static_assert(externalsIndex != meta::InvalidTypeIndex &&
                    externalsIndex >= 0,
                  "Type is not listed on bundle externals list");

    bundle.setExternalsRef(
      externalsIndex,
      std::make_unique<ExternalsRefT<External>>(std::move(externals)));
    return 0;
  }

  std::shared_ptr<BundlesRealm> bundlesRealm_;
  std::vector<std::shared_ptr<IBundleInstaceImpl>> bundles_;
};

} // namespace sb