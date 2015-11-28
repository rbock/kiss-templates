#include "parse_context.h"
#include "line.h"

namespace kiste
{
  namespace
  {
    auto determine_curly_level(const parse_context& ctx) -> std::size_t
    {
      auto level = ctx._curly_level;
      for (const auto& c : ctx._line)
      {
        switch (c)
        {
        case '{':
          ++level;
          break;
        case '}':
          if (level == 0)
            throw parse_error(ctx, "Too many closing curly braces in C++");
          --level;
          break;
        default:
          break;
        }
      }
      return level;
    }

    auto determine_class_curly_level(const parse_context& ctx, const line_t& line) -> std::size_t
    {
      switch (line.type)
      {
      case line_type::class_begin:
        return ctx._curly_level;
      case line_type::class_end:
        return 0;
      default:
        return ctx._class_curly_level;
      }
    }

    auto has_trailing_return(const line_t& line) -> bool
    {
      if (line.type == line_type::text and not line.commands.empty() and
          line.commands.back().type == command_type::trim_trailing_return)
      {
        return false;
      }
      return true;
    }
  }

  auto parse_context::update(const line_t& line) -> void
  {
    _curly_level = determine_curly_level(*this);
    _class_curly_level = determine_class_curly_level(*this, line);
    _has_trailing_return = ::kiste::has_trailing_return(line);
  }
}
