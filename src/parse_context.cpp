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
