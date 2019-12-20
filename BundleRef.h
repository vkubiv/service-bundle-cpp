#pragma once

#include "BundleInstance.h"
#include "TypeList.h"
#include "meta/TypeListUtils.h"


namespace sb
{
    template<class Bundle>
    class BundleRef
    {
    public:
        using Exports = typename Bundle::Exports;
        using Externals = typename Bundle::Externals;        

        explicit BundleRef(IBundleInstaceImpl* impl = 0)
            : impl_(impl) {}

        template<class T>
        ServiceRef<T> getService() const
        {
            constexpr int exportIndex = meta::TypeIndex<T, Exports>::value;
            static_assert(exportIndex != meta::InvalidTypeIndex && exportIndex >= 0, "Type is not listed on bundle export list");

            auto & ref = impl_->getExportRef(exportIndex);
            auto & typedefRef = dynamic_cast<ExportRefT<T>&>(ref);
            return typedefRef.ref;
        }

        std::future< BundleRef<Bundle> > onActive()
        {
            return std::async(std::launch::deferred, [impl = impl_]() -> BundleRef<Bundle> {
                return BundleRef<Bundle>(impl->onActiveFuture().get());
            });
        }

    private:
        IBundleInstaceImpl* impl_;
    };
}