#pragma once

#include "../TypeList.h"

namespace sb {
namespace meta {
constexpr int InvalidTypeIndex = -1;

template<class T, class TypeListHead, class... TypeList>
struct TypeIndexImpl
{
  enum
  {
    value = TypeIndexImpl<T, TypeList...>::value + 1
  };
};

template<class T, class... TypeList>
struct TypeIndexImpl<T, T, TypeList...>
{
  enum
  {
    value = 0
  };
};

template<class T, class TypeListHead>
struct TypeIndexImpl<T, TypeListHead>
{
  enum
  {
    value = InvalidTypeIndex
  };
};

template<class T>
class ONE
{
  enum
  {
    value = 1
  };
};

template<class T, class TypeList>
struct TypeIndex;

template<class T, class... List>
struct TypeIndex<T, TypeList<List...>>
{
  enum
  {
    value = TypeIndexImpl<T, List...>::value
  };
};

template<class TypeList>
struct ListSize;

template<class T, class... List>
struct ListSize<TypeList<T, List...>>
{
  enum
  {
    value = 1 + ListSize<TypeList<List...>>::value
  };
};

template<class T>
struct ListSize<TypeList<T>>
{
  enum
  {
    value = 1
  };
};

template<typename T, int N, int M = 0>
struct TypeListGet;

template<int N, int M, typename T, typename... TT>
struct TypeListGet<TypeList<T, TT...>, N, M>
{
  static_assert(N < (int)sizeof...(TT) + 1 + M, "type index out of bounds");

  using Type =
    std::conditional_t<N == M,
                       T,
                       typename TypeListGet<TypeList<TT...>, N, M + 1>::Type>;
};

template<int N, int M>
struct TypeListGet<TypeList<>, N, M>
{
  using Type = void;
};

template<int N>
struct TypeListGet<TypeList<>, N, 0>
{};

template<typename T, int N>
using TypeListGetType = typename TypeListGet<T, N>::Type;
} // namespace meta
} // namespace sb