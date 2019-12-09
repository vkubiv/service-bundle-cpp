#pragma once

#include "ServiceBundle.h"
#include "BundleInstaceImpl.h"
#include "ActivatorsRegistry.h"
#include <map>
#include <memory>

namespace sb
{

class BundlesLoader
{
public:

    BundlesLoader()
        : bundlesRealm_(std::make_shared<BundlesRealm>())
    {
    }

    template<class Bundle>
    void createBundle()
    {        
         if (bundlesRealm_->hasBundle(Bundle::BundleId))
             throw std::logic_error(std::string("Bundle creating multiple times in the same loader. Bundle id: ") + Bundle::BundleId);

         auto bundleFactory = ActivatorsRegistry::Instance().getFactory(Bundle::BundleId);

         bundlesRealm_->addBundle(Bundle::BundleId, bundleFactory());
    }

    template<class Bundle>
    boost::future<BundleRef<Bundle>> activateBundle()
    {
        auto bundle = bundlesRealm_->getBundle(Bundle::BundleId);
        if (!bundle)
            throw std::logic_error(std::string("Bundle should be created before activation. Bundle id: ") + Bundle::BundleId);

        bundle->activate();
                    
    }
    
private:
    std::shared_ptr<BundlesRealm> bundlesRealm_;
};

}