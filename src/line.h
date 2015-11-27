#pragma once

#include <string>
#include <vector>
#include "command_type.h"
#include "line_type.h"

namespace kiste
{
  struct command_t
  {
    std::size_t end_pos;
    command_type type;
    std::string text;
  };

  struct member_t
  {
    std::string class_name;
    std::string name;
  };

  struct class_t
  {
    std::string name;
    std::string parent_name;
  };

  struct parse_context;

  struct line_t
  {
    std::size_t curly_level = 0;
    line_type previous_type = line_type::none;
    line_type type = line_type::none;
    line_type next_type = line_type::none;
    bool trailing_return = false;
    // depending on the type, one of the following members is to be used
    std::vector<command_t> commands;
    class_t class_data;
    member_t member;

    line_t() = default;

    line_t(line_type t) : type(t)
    {
    }

    line_t(line_type t, const std::vector<command_t>& c) : type(t), commands(c)
    {
    }

    line_t(const class_t& data) : type(line_type::class_begin), class_data(data)
    {
    }

    line_t(const member_t& data) : type(line_type::member), member(data)
    {
    }

    auto update(const parse_context& ctx) -> void;
  };
}
