#ifndef KISS_TEMPLATES_KISTE_RAW_H
#define KISS_TEMPLATES_KISTE_RAW_H

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
  class raw
  {
    std::ostream& _os;

  public:
    raw(std::ostream& os) : _os(os)
    {
    }

    raw() = delete;
    raw(const raw&) = delete;
    raw(raw&&) = default;
    raw& operator=(const raw&) = delete;
    raw& operator=(raw&&) = default;
    ~raw() = default;

    auto text(const char* t) -> void
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
      _os << cr._t;
    }

    template <typename T>
    auto escape(T&& t) -> void
    {
      _os << std::forward<T>(t);
    }
  };
}

#endif
