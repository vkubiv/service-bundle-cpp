#pragma once

namespace sb {

template<class... List>
struct TypeList
{
  const static int size = sizeof...(List);
};

} // namespace sb