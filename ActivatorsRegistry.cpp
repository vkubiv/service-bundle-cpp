#include "ActivatorsRegistry.h"
#include <assert.h>

namespace sb {
ActivatorsRegistry&
ActivatorsRegistry::Instance()
{
  static ActivatorsRegistry instance;
  return instance;
}

bool
ActivatorsRegistry::registerFactory(std::string name, ActivatorFactory factory)
{
  auto isNewInserted = registry_.emplace(name, factory).second;
  assert(isNewInserted);
  return true;
}

const ActivatorFactory&
ActivatorsRegistry::getFactory(const std::string& name) const
{
  auto i = registry_.find(name);
  if (i == registry_.end())
    throw std::logic_error("Invalid bundle name: " + name);

  return i->second;
}
} // namespace sb