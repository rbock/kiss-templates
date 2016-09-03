#ifndef KISS_TEMPLATES_KISTE_HTML_H
#define KISS_TEMPLATES_KISTE_HTML_H

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

#include <ostream>

#include <kiste/raw_type.h>

namespace kiste
{
  class html
  {
    std::ostream& _os;

  public:
    html(std::ostream& os) : _os(os)
    {
    }

    html() = delete;
    html(const html&) = default;
    html(html&&) = default;
    html& operator=(const html&) = default;
    html& operator=(html&&) = default;
    ~html() = default;

    auto text(const char* text) -> void
    {
      _os << text;
    }

    auto escape(const char& c) -> void
    {
      switch (c)
      {
      case '<':
        _os << "&lt;";
        break;
      case '>':
        _os << "&gt;";
        break;
      case '\'':
        _os << "&#39;";
        break;
      case '"':
        _os << "&quot;";
        break;
      case '&':
        _os << "&amp;";
        break;
      default:
        _os << c;
      }
    }

    template <typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
    auto escape(const T& t) -> void
    {
      _os << t;
    }

    template <typename T,
              typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
    auto escape(const T& t) -> void
    {
      _os << t;
    }

    template <typename T>
    auto escape(const raw_t<T>& r) -> void
    {
      _os << r._t;
    }

    template <typename T>
    auto escape(const conditionally_raw_t<T>& cr) -> void
    {
      if (cr._isRaw)
        _os << cr._t;
      else
        escape(cr._t);
    }

    template <typename T,
              typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = nullptr>
    auto escape(const T& t) -> void
    {
      for (const auto& c : std::string(t))  // maybe specialize for char* to avoid the constructor?
      {
        escape(c);
      }
    }

    template <typename T>
    auto raw(T&& t) -> void
    {
      _os << std::forward<T>(t);
    }
  };
}

#endif
