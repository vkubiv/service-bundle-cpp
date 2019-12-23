#pragma once

#include "../BundleRef.h"
#include "../ServiceRef.h"
#include "../TypeList.h"

#include <memory>
#include <type_traits>

namespace sb {
namespace meta {

template<class TInjector, class TParent>
struct any_type
{
  template<class T>
  constexpr operator ServiceRef<T>()
  {
    return injector_.template create<T>();
  }

  template<class T>
  constexpr operator BundleRef<T>()
  {
    return injector_.template getBundleRef<T>();
  }

  any_type(TInjector& injector)
    : injector_(injector)
  {}

  const TInjector& injector_;
};

constexpr auto BOOST_DI_CFG_CTOR_LIMIT_SIZE = 10;

class DummyInjector
{};

template<class TInjector, class T, std::size_t>
using any_type_t = any_type<TInjector, T>;

template<class...>
struct is_constructible;

template<class T, std::size_t... Ns>
struct is_constructible<T, std::index_sequence<Ns...>>
  : std::is_constructible<T, any_type_t<DummyInjector, T, Ns>...>
{};

template<class T, std::size_t... Ns>
constexpr auto
get_ctor_size(std::index_sequence<Ns...>) noexcept
{
  auto value = 0;
  int _[] = { 0,
              (is_constructible<T, std::make_index_sequence<Ns>>{}
                 ? value = Ns
                 : value)... };
  return value;
}

template<class T, class TInjector, std::size_t... Ns>
T*
ConstructTypeImpl(TInjector& injector, std::index_sequence<Ns...>)
{
  return new T(any_type_t<TInjector, T, Ns>(injector)...);
}

template<class T, class TInjector>
std::unique_ptr<T>
ConstructType(TInjector& injector)
{
  constexpr auto size =
    get_ctor_size<T>(std::make_index_sequence<BOOST_DI_CFG_CTOR_LIMIT_SIZE>{});

  return std::unique_ptr<T>(
    ConstructTypeImpl<T>(injector, std::make_index_sequence<size>()));
}
} // namespace meta
} // namespace sb
