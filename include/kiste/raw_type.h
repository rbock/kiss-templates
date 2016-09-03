#ifndef KISS_TEMPLATES_KISTE_RAW_TYPE_H
#define KISS_TEMPLATES_KISTE_RAW_TYPE_H

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

namespace kiste
{
  template <typename T>
  struct raw_t
  {
    // Additionally allow implicit construction from `const char*` if T convertible to std::string
    raw_t(typename std::enable_if<std::is_convertible<T, std::string>::value, const char*>::type s)
        : _t(s)
    {
    }

    raw_t(const T& t) : _t(t)
    {
    }

    T _t;
  };

  template <typename T>
  auto rawval(T&& t) -> raw_t<
      typename std::enable_if<!std::is_convertible<T, std::string>::value, std::string>::type>
  {
    return raw_t<T>(std::forward<T>(t));
  }

  template <typename T>
  auto rawval(T&& t) -> raw_t<
      typename std::enable_if<std::is_convertible<T, std::string>::value, std::string>::type>
  {
    return raw_t<std::string>(std::forward<T>(t));
  }

  template <typename T>
  struct conditionally_raw_t
  {
    // Additionally allow implicit construction from `const char*` if T convertible to std::string
    conditionally_raw_t(
        typename std::enable_if<std::is_convertible<T, std::string>::value, const char*>::type s)
        : _t(s), _isRaw(false)
    {
    }

    conditionally_raw_t(const T& t) : _t(t), _isRaw(false)
    {
    }

    conditionally_raw_t(const T& t, bool isRaw) : _t(t), _isRaw(isRaw)
    {
    }

    conditionally_raw_t(const raw_t<T>& r) : _t(r._t), _isRaw(true)
    {
    }

    T _t;
    bool _isRaw;
  };

  // Most common use cases
  using conditionally_raw_string = conditionally_raw_t<std::string>;
  using raw_string = raw_t<std::string>;
}

#endif
