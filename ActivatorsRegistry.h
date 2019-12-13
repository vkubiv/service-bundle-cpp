#pragma once

#include "ServiceBundle.h"
#include "BundleInstaceImpl.h"
#include <functional>
#include <memory>
#include <map>


namespace sb
{

using ActivatorFactory = std::function< std::unique_ptr<IBundleInstaceImpl> ()>;

class ActivatorsRegistry
{
public:

    static ActivatorsRegistry& Instance();
    bool registerFactory(std::string name, ActivatorFactory factory);

    const ActivatorFactory & getFactory(const std::string& name) const;

private:
    std::map<std::string, ActivatorFactory> registry_;
};

template<class Activator>
bool ExportBundleActivator()
{
    ActivatorsRegistry::Instance()
        .registerFactory(Activator::BundleId, []() 
        {
            auto p = new BundleInstaceImplT<Activator>();
            std::unique_ptr<IBundleInstaceImpl> ptr (p);
            return ptr;
        });

    return true;
}
    
}