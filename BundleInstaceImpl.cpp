#include "BundleInstaceImpl.h"

namespace sb
{

    struct AAA
    {
    };

    struct AA
    {
        AA(ServiceRef<AAA> a){}
    };

    struct A{
        A(ServiceRef<AA> a){}
    };
    class B{};
    class C{};

    using ProvidersList = TypeList<Provider<A>, Provider<AA>, Provider<AAA>, Provider<B>, Provider<C> >;
       

    void test()
    {

        using TestProviders = ProvidersSet<ProvidersList>;
        TestProviders testProviders;
        Injector<TestProviders> i(testProviders);

        testProviders.activateProviders (i);

        auto &a = *testProviders.getDependency<A>(i);
        

        //ExportRefT<A> r (a);
    }
}