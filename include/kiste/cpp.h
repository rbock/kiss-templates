#ifndef KISS_TEMPLATES_KISTE_CPP_H
#define KISS_TEMPLATES_KISTE_CPP_H

#include <ostream>

namespace kiste
{
  class cpp
  {
    std::ostream& _os;

  public:
    cpp(std::ostream& os) : _os(os)
    {
    }

    cpp() = delete;
    cpp(const cpp&) = default;
    cpp(cpp&&) = default;
    cpp& operator=(const cpp&) = default;
    cpp& operator=(cpp&&) = default;
    ~cpp() = default;

    auto text(const char* text) -> void
    {
      _os << text;
    }

    auto escape(const char& c) -> void
    {
      switch (c)
      {
      case '\\':
        _os << "\\\\";
        break;
      case '"':
        _os << "\\\"";
        break;
      case '\n':
        _os << "\\n";
        break;
      default:
        _os << c;
      }
    }

    template <typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
    auto escape(const T& t) -> void
    {
      _os << t;
    }

    template <typename T,
              typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
    auto escape(const T& t) -> void
    {
      _os << t;
    }

    template <typename T,
              typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = nullptr>
    auto escape(const T& t) -> void
    {
      for (const auto& c : std::string(t))  // maybe specialize for char* to avoid the constructor?
      {
        escape(c);
      }
    }

    template <typename T>
    auto raw(T&& t) -> void
    {
      _os << std::forward<T>(t);
    }
  };
}

#endif
