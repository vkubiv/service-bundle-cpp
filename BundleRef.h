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

        BundleRef(IBundleInstaceImpl* impl)
            : impl_(impl) {}

        template<class T>
        T& getService() const
        {
            constexpr int exportIndex = meta::TypeIndex<T, Exports>::value;
            static_assert(exportIndex != meta::InvalidTypeIndex && exportIndex >= 0, "Type is not listed on bundle export list");

            auto & ref = impl_->getExportRef(exportIndex);
            auto & typedefRef = dynamic_cast<ExportRefT<T>&>(ref);
            return *typedefRef.ref;
        }

        boost::shared_future< BundleRef<Bundle> > onActive()
        {
            return impl_->onActiveFuture().then([](auto impl) {
                return BundleRef<Bundle>(impl.get());
            });
        }

    private:
        IBundleInstaceImpl*  impl_;
    };
}