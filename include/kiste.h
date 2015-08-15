#ifndef KISTE_H
#define KISTE_H

#include <ostream>

namespace kiste
{
  struct terminal_t
  {
  };
  constexpr auto terminal = terminal_t{};

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
