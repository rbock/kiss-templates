#include "line.h"
#include "parse_context.h"

namespace kiste
{
  auto line_t::update(const parse_context& ctx) -> void
  {
    _curly_level = ctx._curly_level;
    if (_segments.empty())
    {
      _segments.push_back({0, segment_type::text, ""});
    }
    if (_type == line_type::text and _segments.back()._type == segment_type::trim_trailing_return)
    {
      _segments.pop_back();
    }
    if (ctx._has_trailing_return)
    {
      if (not _segments.empty() and _segments.back()._type == segment_type::text)
      {
        _segments.back()._text.push_back('\n');
      }
    }
    _trailing_return = false;
#warning get rid of this
  }
}
