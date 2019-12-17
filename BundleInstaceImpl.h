#pragma once

#include "ServiceBundle.h"
#include "ServiceRef.h"
#include "BundleRef.h"
#include "BundleInstance.h"
#include "meta/Constructor.h"
#include <vector>
#include <memory>
#include <type_traits>
#include <mutex>
#include <map>

namespace sb
{    

    class BundlesRealm
    {
    public:
        
        bool hasBundle(const char * bundleId) const
        {
            return bundles_.find(bundleId) != bundles_.end();
        }

        void addBundle(const char * bundleId, std::shared_ptr<IBundleInstaceImpl> bundle)
        {
            bundles_[bundleId] = bundle;
        }

        std::shared_ptr<IBundleInstaceImpl> getBundle(const char * bundleId)
        {
            auto bundle = bundles_.find(bundleId);
            if (bundle == bundles_.end())
                return std::shared_ptr<IBundleInstaceImpl>();

            return bundle->second;
        }

    private:
        struct CmpStr
        {
           bool operator()(char const *a, char const *b) const
           {
              return std::strcmp(a, b) < 0;
           }
        };

        std::map<char const *, std::shared_ptr<IBundleInstaceImpl> > bundles_;
    };

    template<class Providers, int N>
    struct ProvidersSetElement
    {
        using Type = meta::TypeListGetType<Providers, N>;
        Type provider;
    };

    template<class, class >
    struct ProvidersSetBase;

    template<typename... ProviderTT, int... NN>
    struct ProvidersSetBase<TypeList<ProviderTT...>, std::integer_sequence<int, NN...> > 
        : ProvidersSetElement<TypeList<ProviderTT...>, NN> ...
    { 
        using Interfaces = TypeList<typename ProviderTT::Interface ...>;

        template<class I>
        constexpr static int GetTypeIndex()
        {
            return meta::TypeIndex<I, Interfaces>::value;
        }
        
        template<class I, class Injector>
        ServiceRef<I> getDependency(Injector& injector)
        {
           return ProvidersSetElement<TypeList<ProviderTT...>, GetTypeIndex<I>() >::provider
                .get(injector);
        }

    private:
        template<class I, class Injector>
        int callBeforeBundleActivate(Injector& injector)
        {
           ProvidersSetElement<TypeList<ProviderTT...>, GetTypeIndex<I>() >::provider
                .beforeBundleActivate(injector);
           return 0;
        }

        template<class I>
        int callAfterBundleActivate()
        {
           ProvidersSetElement<TypeList<ProviderTT...>, GetTypeIndex<I>() >::provider
                .afterBundleActivate();
           return 0;
        }
    public:
        template<class Injector>
        void beforeBundleActivate(Injector& injector)
        {
            const int _[] = {
                0,
                callBeforeBundleActivate<typename ProviderTT::Interface>(injector)...
            };
        }

        void afterBundleActivate()
        {
            const int _[] = {
                0,
                callAfterBundleActivate<typename ProviderTT::Interface>()...
            };
        }
    };
    

    template<class ProvidersList>
    struct ProvidersSet :  ProvidersSetBase<ProvidersList, std::make_integer_sequence<int, ProvidersList::size>>
    {  
    };

    template<class ReferencesList>
    class ReferencesSet;

    template<class... References>
    class ReferencesSet<TypeList<References...>>
    {
    public:

        void linkReferences(BundlesRealm& realm)
        {
            references_ = {
                realm.getBundle(References::BundleId).get()
                ...
            };
        }

        template<class Bundle>
        BundleRef<Bundle> getReference()
        {
            constexpr int referenceIndex = meta::TypeIndex<Bundle, TypeList<References...>>::value;
            static_assert(referenceIndex != meta::InvalidTypeIndex, "Bundle reference is not listed on bundle references list");

            if (0 <= referenceIndex && referenceIndex < references_.size())
            {
                return BundleRef<Bundle>(references_[referenceIndex]);                
            }
            throw std::logic_error("getReference: referenceIndex out of bounds");
        }

    private:
        std::vector<IBundleInstaceImpl* > references_;
    };

    template<class ProvidersSetT, class BundleReferencesSetT>
    class Injector
    {
    public:
        template<class T>
        ServiceRef<T> create() const
        {
           return providersSet_.getDependency<T>(*this);
        }

        template<class T>
        BundleRef<T> getBundleRef() const
        {
           return bundleReferencesSet_.getReference<T>();
        }

        Injector(ProvidersSetT & providersSet, BundleReferencesSetT & bundleReferencesSet)
            : providersSet_(providersSet)
            , bundleReferencesSet_(bundleReferencesSet) {}

    private:
        ProvidersSetT & providersSet_;
        BundleReferencesSetT & bundleReferencesSet_;
    };
        

    template<class BundleActivator>
    class BundleInstaceImplT : public IBundleInstaceImpl
    {
        using ExportsTable = std::vector<std::shared_ptr<ExportRef>>;
        using ExternalsTable = std::vector<std::unique_ptr<ExternalsRef> >;
        using BundleInstaceImpl = BundleInstaceImplT<BundleActivator>;
    public:

        using Bundle = typename BundleActivator::Bundle;
        using Exports = typename Bundle::Exports;
        using Externals = typename Bundle::Externals;
        using Providers = typename BundleActivator::Providers;
        using References = typename BundleActivator::References;        

        using BundleProvidersSet = ProvidersSet<Providers>;
        using BundleReferencesSet = ReferencesSet<References>;
        using BundleInjector = Injector<BundleProvidersSet, BundleReferencesSet>;

        BundleInstaceImplT()
            : externalsTable_(Externals::size)
            , bundleInjector_(bundleProvidersSet_, bundleReferencesSet_)
        {
            onActivatedFuture_ = onActivatedPromise_.get_future().share();
        }

        virtual ExportRef & getExportRef(uint32_t exportIndex) override
        {
            if (0 <= exportIndex && exportIndex < exportsTable_.size())
            {
                if (auto ptr = exportsTable_[exportIndex])
                {
                    return *ptr;
                }
                throw std::logic_error("exportsTable_ is not properly initialized");
            }
            throw std::logic_error("exportIndex out of bounds");
        }

        virtual void setExternalsRef(uint32_t externalsIndex, std::unique_ptr<ExternalsRef> externalsRef) override
        {
            if (0 <= externalsIndex && externalsIndex < externalsTable_.size())
            {
                externalsTable_[externalsIndex] = std::move(externalsRef);
            }
            else
            {
                throw std::logic_error("setExternalsRef: externalsIndex out of bounds");
            }
        }
               

        template<class T>
        ServiceRef<T> getService()
        {
            return bundleInjector_.create<T>();
        }
        
        template<class External>
        External & getExternal()
        {
            constexpr int externalsIndex = meta::TypeIndex<External, Externals>::value;
            static_assert(externalsIndex != meta::InvalidTypeIndex, "Type is not listed on bundle extrenals list");

            if (0 <= externalsIndex && externalsIndex < externalsTable_.size())
            {
                ExternalsRef external = externalsTable_[externalsIndex].get();
                auto & typedefRef = dynamic_cast<ExternalsRefT<External>&>(*external);
                return typedefRef.ref;
            }
            throw std::logic_error("getExternal: externalsIndex out of bounds");

        }

        template<class Bundle>
        BundleRef<Bundle> getReference()
        {
            return bundleReferencesSet_.getReference<Bundle>();
        }

        /*
        Bundle lifecycle events
        */
        virtual void linkReferences(BundlesRealm& realm) override
        {
            bundleReferencesSet_.linkReferences(realm);
        }
        virtual void createServices()  override
        {
            bundleProvidersSet_.beforeBundleActivate (bundleInjector_);
            createExportTalbe(Exports());
        }
        virtual AsyncActivateResult activate() override;

        virtual std::shared_future<IBundleInstaceImpl*> onActiveFuture() override
        {
            std::lock_guard<std::mutex> lock(futureCopyMutex_);
            auto onActivatedFutureCopy = onActivatedFuture_;
            return onActivatedFutureCopy;
        }

    private:

        template<class T>
        std::shared_ptr<ExportRef> createExportRef()
        {
            return std::make_shared<ExportRefT<T>>(bundleInjector_.create<T>());
        }


        template<class... TT>
        void createExportTalbe(TypeList<TT...> exports)
        {
            exportsTable_ = ExportsTable({
                createExportRef<TT>()...
            });
        }

    private:
        ExportsTable exportsTable_;
        ExternalsTable externalsTable_;
        BundleProvidersSet bundleProvidersSet_;
        BundleReferencesSet bundleReferencesSet_;
        BundleInjector bundleInjector_;
        BundleActivator bundleActivator_;

        std::promise<IBundleInstaceImpl*> onActivatedPromise_;
        std::shared_future<IBundleInstaceImpl*> onActivatedFuture_;
        std::mutex futureCopyMutex_;
    };

    template<class BundleActivator>
    class ThisBundle
    {        
        using BundleInstaceImpl = BundleInstaceImplT<BundleActivator>;
    public:
        ThisBundle(BundleInstaceImpl* bundleInstaceImpl)
            : bundleInstaceImpl_(bundleInstaceImpl)
        {
        }

        template<class T>
        T& getService()
        {
            return *bundleInstaceImpl_->getService<T>().getUnsafe();
        }

        template<class External>
        External & getExternal();

        template<class Bundle>
        std::shared_future< BundleRef<Bundle> > onActive()
        {
            return bundleInstaceImpl_->getReference<Bundle>()
                .onActive();
        }        

    private:
        BundleInstaceImpl * bundleInstaceImpl_;
    };

    template<class BundleActivator>
    AsyncActivateResult BundleInstaceImplT<BundleActivator>::activate()
    {
        return std::async(std::launch::deferred, [this](){
            bundleActivator_.activate(ThisBundle<BundleActivator>(this)).get();
            bundleProvidersSet_.afterBundleActivate();
            onActivatedPromise_.set_value(this);
        });
    }

    template<class T>
    struct BundleActivator
    {
        using Bundle = T;
        using Exports = typename Bundle::Exports;
        using Externals = typename Bundle::Externals;        
        static constexpr auto BundleId = Bundle::BundleId;       
    };

    //BundlesRegistry
}