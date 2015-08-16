#ifndef KISS_TEMPLATES_KISTE_H
#define KISS_TEMPLATES_KISTE_H

#include <kiste/terminal.h>
#include <kiste/raw.h>

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
