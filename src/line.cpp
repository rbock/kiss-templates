#include "line.h"
#include "parse_context.h"

namespace kiste
{
  auto line_t::enforce_at_least_one_segment() -> void
  {
    if (_segments.empty())
    {
      _segments.push_back({0, segment_type::text, ""});
    }
  }

  auto line_t::enforce_trailing_text_segment() -> void
  {
    enforce_at_least_one_segment();
    if (_segments.back()._type != segment_type::text)
    {
      _segments.push_back({0, segment_type::text, ""});
    }
  }

  auto line_t::add_character(const char c) -> void
  {
    enforce_trailing_text_segment();
    _segments.back()._text.push_back(c);
  }

  auto line_t::add_segment(const segment_t& segment) -> size_t
  {
    switch (segment._type)
    {
    case segment_type::text:
      for (const auto c : segment._text)
      {
        add_character(c);
      }
      break;
    default:
      _segments.push_back(segment);
      break;
    }

    return segment._end_pos;
  }

  auto line_t::finish(const parse_context& ctx) -> void
  {
    _curly_level = ctx._curly_level;
    if (_type == line_type::text)
    {
      enforce_at_least_one_segment();
      if (_segments.back()._type == segment_type::trim_trailing_return)
      {
        _segments.pop_back();
        enforce_at_least_one_segment();
      }
      if (ctx._has_trailing_return)
      {
        enforce_trailing_text_segment();
        _segments.back()._text.push_back('\n');
      }
      _trailing_return = false;
#warning get rid of this
    }
  }
}
