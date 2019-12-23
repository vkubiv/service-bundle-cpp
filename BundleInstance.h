#pragma once

#include <future>

namespace sb {
class BundlesRealm;

using AsyncActivateResult = std::future<void>;

inline AsyncActivateResult
make_ready_future()
{
  std::promise<void> readyPromiss;
  readyPromiss.set_value();
  return readyPromiss.get_future();
}

template<class T, class E>
AsyncActivateResult
make_exceptional_future(E ex)
{
  std::promise<T> readyPromiss;
  readyPromiss.set_exception(ex);
  return readyPromiss.get_future();
}

struct ExportRef
{
  virtual ~ExportRef() {}
};

template<class T>
struct ExportRefT : public ExportRef
{

  ExportRefT(ServiceRef<T> the_ref)
    : ref(the_ref)
  {}

  ServiceRef<T> ref;
};

struct ExternalsRef
{
  virtual ~ExternalsRef() {}
};

template<class T>
struct ExternalsRefT : public ExternalsRef
{

  ExternalsRefT(T the_ref)
    : ref(std::move(the_ref))
  {}

  T ref;
};

class IBundleInstaceImpl
{
public:
  virtual ~IBundleInstaceImpl() {}
  virtual ExportRef& getExportRef(uint32_t exportIndex) = 0;
  virtual void setExternalsRef(uint32_t exportIndex,
                               std::unique_ptr<ExternalsRef>) = 0;

  /*
  Bundle lifecycle events
  */
  virtual void linkReferences(BundlesRealm&) = 0;
  virtual void createServices() = 0;
  virtual AsyncActivateResult activate() = 0;

  virtual std::shared_future<IBundleInstaceImpl*> onActiveFuture() = 0;
};
} // namespace sb