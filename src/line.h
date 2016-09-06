#pragma once
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

#include <string>
#include <vector>
#include "segment_type.h"
#include "line_type.h"

namespace kiste
{
  struct segment_t
  {
    std::size_t _end_pos;
    segment_type _type;
    std::string _text;
  };

  struct member_t
  {
    std::string class_name;
    std::string name;
  };

  struct class_t
  {
    std::string _name;
    std::string _parent_name;
  };

  struct parse_context;

  struct line_data_t
  {
    line_type _type = line_type::none;
    // depending on the type, one of the following members is to be used
    std::vector<segment_t> _segments;
    class_t _class_data;
    member_t _member;

    line_data_t() = default;
    line_data_t(const line_data_t&) = default;
    line_data_t(line_data_t&&) = default;

    line_data_t(line_type type) : _type(type)
    {
    }

    line_data_t(line_type type, const std::vector<segment_t>& segments)
        : _type(type), _segments(segments)
    {
    }

    line_data_t(const class_t& data) : _type(line_type::class_begin), _class_data(data)
    {
    }

    line_data_t(const member_t& data) : _type(line_type::member), _member(data)
    {
    }

    auto add_character(const char c) -> void;
    auto add_segment(const segment_t& segment) -> size_t;

  protected:
    auto enforce_at_least_one_segment() -> void;
    auto enforce_trailing_text_segment() -> void;
  };

  struct line_t : public line_data_t
  {
    std::size_t _curly_level = 0;
    bool _previous_line_ends_with_text = false;
    bool _next_line_starts_with_text = false;

    line_t(const parse_context& ctx, const line_data_t& line_data);

    auto ends_with_text() const -> bool;
    auto starts_with_text() const -> bool;
  };
}
