#ifndef KISS_TEMPLATES_KISTE_HTML_H
#define KISS_TEMPLATES_KISTE_HTML_H

#include <ostream>

namespace kiste
{
  struct html
  {
    std::ostream& _os;

		html(std::ostream& os):
			_os(os)
		{}

		html() = delete;
		html(const html&) = default;
		html(html&&) = default;
		html& operator=(const html&) = default;
		html& operator=(html&&) = default;
		~html() = default;

		auto get_ostream() -> std::ostream&
		{
			return _os;
		}

    auto operator()(const char& c) const -> void
    {
			switch (c)
			{
			case '<':
				_os <<  "&lt;";
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
    auto operator()(const T& t) const -> void
    {
      _os << t;
    }

    template <typename T, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
    auto operator()(const T& t) const -> void
    {
      _os << t;
    }

    template <typename T, typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = nullptr>
    auto operator()(const T& t) const -> void
    {
			for (const auto& c : std::string(t)) // maybe specialize for char* to avoid the constructor?
			{
				operator()(c);
			}
    }
  };
}

#endif
