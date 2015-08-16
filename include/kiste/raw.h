#ifndef KISS_TEMPLATES_KISTE_H
#define KISS_TEMPLATES_KISTE_H

#include <ostream>

namespace kiste
{
  struct raw
  {
    std::ostream& _os;

    template <typename T>
    auto operator()(T&& t) const -> void
    {
      _os << std::forward<T>(t);
    }
  };
}

#endif
