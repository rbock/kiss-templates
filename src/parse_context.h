#pragma once

#include <stdexcept>
#include <iostream>
#include <string>

namespace kiste
{
  struct line_t;

  struct parse_context
  {
    std::istream& is;
    std::ostream& os;
    std::string filename;
    bool report_exceptions = false;
    std::string line;
    std::size_t line_no = 0;
    std::size_t curly_level = 0;
    std::size_t class_curly_level = 0;
    bool has_trailing_return = false;

    parse_context(std::istream& is_,
                  std::ostream& os_,
                  const std::string& filename_,
                  bool report_exceptions_)
        : is(is_), os(os_), filename{filename_}, report_exceptions{report_exceptions_}
    {
    }

    auto update(const line_t& l) -> void;
  };

  struct parse_error : public std::runtime_error
  {
    std::string filename;
    std::string line;
    std::size_t line_no = 0;

    parse_error(const parse_context& ctx, const std::string& message)
        : std::runtime_error{message}, filename{ctx.filename}, line{ctx.line}, line_no{ctx.line_no}
    {
    }
  };
}
