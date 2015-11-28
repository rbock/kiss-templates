#pragma once

#include <stdexcept>
#include <iostream>
#include <string>

namespace kiste
{
  struct line_t;

  struct parse_context
  {
    std::istream& _is;
    std::ostream& _os;
    std::string _filename;
    bool _report_exceptions = false;
    std::string _line;
    std::size_t _line_no = 0;
    std::size_t _curly_level = 0;
    std::size_t _class_curly_level = 0;
    bool _has_trailing_return = false;

    parse_context(std::istream& is,
                  std::ostream& os,
                  const std::string& filename,
                  bool report_exceptions)
        : _is(is), _os(os), _filename{filename}, _report_exceptions{report_exceptions}
    {
    }

    auto update(const line_t& line) -> void;
  };

  struct parse_error : public std::runtime_error
  {
    using std::runtime_error::runtime_error;
  };
}
