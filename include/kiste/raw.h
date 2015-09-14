#ifndef KISS_TEMPLATES_KISTE_RAW_H
#define KISS_TEMPLATES_KISTE_RAW_H

#include <ostream>

namespace kiste
{
  class raw
  {
    std::ostream& _os;

  public:
    raw(std::ostream& os) : _os(os)
    {
    }

    raw() = delete;
    raw(const raw&) = delete;
    raw(raw&&) = default;
    raw& operator=(const raw&) = delete;
    raw& operator=(raw&&) = default;
    ~raw() = default;

    auto text(const char* t) -> void
    {
      _os << t;
    }

    template <typename T>
    auto escape(T&& t) -> void
    {
      _os << std::forward<T>(t);
    }
  };
}

#endif
