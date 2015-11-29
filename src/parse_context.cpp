#include "parse_context.h"
#include "line.h"

namespace kiste
{
  namespace
  {
    auto determine_curly_level(const parse_context& ctx, const line_data_t& line_data)
        -> std::size_t
    {
      auto level = ctx._curly_level;
      if (line_data._type != line_type::cpp)
      {
        return level;
      }

      for (const auto& c : ctx._line)
      {
        switch (c)
        {
        case '{':
          ++level;
          break;
        case '}':
          if (level == 0)
            throw parse_error("Too many closing curly braces in C++");
          --level;
          break;
        default:
          break;
        }
      }
      return level;
    }

    auto determine_class_curly_level(const parse_context& ctx, const line_data_t& line_data)
        -> std::size_t
    {
      switch (line_data._type)
      {
      case line_type::class_begin:
        return ctx._curly_level;
      case line_type::class_end:
        return 0;
      default:
        return ctx._class_curly_level;
      }
    }

    auto has_trailing_return(const line_data_t& line_data) -> bool
    {
      if (line_data._type == line_type::text and not line_data._segments.empty() and
          line_data._segments.back()._type == segment_type::trim_trailing_return)
      {
        return false;
      }
      return true;
    }
  }

  auto parse_context::update(const line_data_t& line_data) -> void
  {
    _curly_level = determine_curly_level(*this, line_data);
    _class_curly_level = determine_class_curly_level(*this, line_data);
    _has_trailing_return = ::kiste::has_trailing_return(line_data);
  }
}
