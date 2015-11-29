/*
 * Copyright (c) 2015-2015, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "line.h"
#include "parse_context.h"

namespace kiste
{
  auto line_data_t::enforce_at_least_one_segment() -> void
  {
    if (_segments.empty())
    {
      _segments.push_back({0, segment_type::text, ""});
    }
  }

  auto line_data_t::enforce_trailing_text_segment() -> void
  {
    enforce_at_least_one_segment();
    if (_segments.back()._type != segment_type::text)
    {
      _segments.push_back({0, segment_type::text, ""});
    }
  }

  auto line_data_t::add_character(const char c) -> void
  {
    enforce_trailing_text_segment();
    _segments.back()._text.push_back(c);
  }

  auto line_data_t::add_segment(const segment_t& segment) -> size_t
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

  line_t::line_t(const parse_context& ctx, const line_data_t& line_data) : line_data_t(line_data)
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
    }
  }

  auto line_t::ends_with_text() const -> bool
  {
    if (_type != line_type::text)
    {
      return false;
    }
    return _segments.back()._type == segment_type::text;
  }

  auto line_t::starts_with_text() const -> bool
  {
    if (_type != line_type::text)
    {
      return false;
    }
    return _segments.front()._type == segment_type::text;
  }
}
