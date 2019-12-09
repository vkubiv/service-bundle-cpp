#pragma once


#define BOOST_THREAD_VERSION 4
#include <boost/thread/future.hpp>

namespace sb
{
	class BundlesRealm;

	using AsyncActivateResult = boost::future<void>;

    struct ExportRef {
        virtual ~ExportRef() = 0 {}
    };

    template<class T>
    struct ExportRefT : public ExportRef {

        ExportRefT(T* the_ref)
            : ref(the_ref) {}

        T* ref;
    };

    struct ExternalsRef {
        virtual ~ExternalsRef() = 0 {}
    };

    template<class T>
    struct ExternalsRefT : public ExternalsRef {

        ExternalsRefT(T the_ref)
            : ref(std::move(the_ref)) {}

        T ref;
    };

    class BundleInstaceImpl {
    public:
        virtual ~BundleInstaceImpl() {}
        virtual ExportRef & getExportRef(uint32_t exportIndex) = 0;
        virtual void setExternalsRef(uint32_t exportIndex, std::unique_ptr<ExternalsRef>) = 0;

        virtual void linkWithReferencedBundles(BundlesRealm&) = 0;
        virtual AsyncActivateResult activate() = 0;
    };
}