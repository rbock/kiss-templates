#ifndef KISS_TEMPLATES_KISTE_RAW_H
#define KISS_TEMPLATES_KISTE_RAW_H

#include <ostream>

namespace kiste
{
  struct raw
  {
    std::ostream& _os;

		raw(std::ostream& os):
			_os(os)
		{}

		raw() = delete;
		raw(const raw&) = default;
		raw(raw&&) = default;
		raw& operator=(const raw&) = default;
		raw& operator=(raw&&) = default;
		~raw() = default;

		auto get_ostream() -> std::ostream&
		{
			return _os;
		}

    template <typename T>
    auto operator()(T&& t) const -> void
    {
      _os << std::forward<T>(t);
    }
  };
}

#endif
