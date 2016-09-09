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

#include <stdexcept>
#include <iostream>
#include <string>

namespace kiste
{
  struct line_data_t;

  struct parse_context
  {
    std::istream& _is;
    std::ostream& _os;
    std::string _filename;
    bool _report_exceptions = false;
    bool _line_directives = true;
    std::string _line;
    std::size_t _line_no = 0;
    std::size_t _curly_level = 0;
    std::size_t _class_curly_level = 0;
    bool _has_trailing_return = false;

    parse_context(std::istream& is,
                  std::ostream& os,
                  const std::string& filename,
                  bool report_exceptions,
                  bool line_directives)
        : _is(is),
          _os(os),
          _filename{filename},
          _report_exceptions{report_exceptions},
          _line_directives{line_directives}
    {
    }

    auto update(const line_data_t& line) -> void;
  };

  struct parse_error : public std::runtime_error
  {
		parse_error(const std::string& what_arg): std::runtime_error(what_arg){}
		parse_error(const char* what_arg): std::runtime_error(what_arg){}
  };
}
