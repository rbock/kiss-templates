#pragma once

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

  struct line_t
  {
    std::size_t _curly_level = 0;
    bool _previous_line_ends_with_text = false;
    line_type _type = line_type::none;
    bool _next_line_starts_with_text = false;
    bool _trailing_return = false;
    // depending on the type, one of the following members is to be used
    std::vector<segment_t> _segments;
    class_t _class_data;
    member_t _member;

    line_t() = default;

    line_t(line_type type) : _type(type)
    {
    }

    line_t(line_type type, const std::vector<segment_t>& segments)
        : _type(type), _segments(segments)
    {
    }

    line_t(const class_t& data) : _type(line_type::class_begin), _class_data(data)
    {
    }

    line_t(const member_t& data) : _type(line_type::member), _member(data)
    {
    }

    auto add_character(const char c) -> void;
    auto add_segment(const segment_t& segment) -> size_t;

    auto finish(const parse_context& ctx) -> void;

  private:
    auto enforce_at_least_one_segment() -> void;
    auto enforce_trailing_text_segment() -> void;
  };
}
