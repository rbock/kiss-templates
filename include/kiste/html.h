#ifndef KISS_TEMPLATES_KISTE_HTML_H
#define KISS_TEMPLATES_KISTE_HTML_H

#include <ostream>

namespace kiste
{
  class html
  {
    std::ostream& _os;

  public:
    html(std::ostream& os) : _os(os)
    {
    }

    html() = delete;
    html(const html&) = default;
    html(html&&) = default;
    html& operator=(const html&) = default;
    html& operator=(html&&) = default;
    ~html() = default;

    auto text(const char* text) -> void
    {
      _os << text;
    }

    auto escape(const char& c) -> void
    {
      switch (c)
      {
      case '<':
        _os << "&lt;";
        break;
      case '>':
        _os << "&gt;";
        break;
      case '\'':
        _os << "&#39;";
        break;
      case '"':
        _os << "&quot;";
        break;
      case '&':
        _os << "&amp;";
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
