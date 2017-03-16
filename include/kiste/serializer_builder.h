#ifndef KISS_TEMPLATES_KISTE_SERIALIZER_BUILDER_H
#define KISS_TEMPLATES_KISTE_SERIALIZER_BUILDER_H

/*
 * Copyright (c) 2017-2017, Andrei Ivanitckii
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

#include <utility>

namespace kiste
{
  namespace serializer_impl
  {
    template <typename... Policies>
    struct serializer_base;

    template <>
    struct serializer_base<>
    {
      template <typename T>
      void escape()
      {
        static_assert(sizeof(T) < 0, "Implementation without arguments is not intended to be used");
      }
    };

    template <typename Policy, typename... Rest>
    struct serializer_base<Policy, Rest...> : Policy, serializer_base<Rest...>
    {
      using RestBase = serializer_base<Rest...>;

      using Policy::escape;
      using RestBase::escape;

      serializer_base(Policy&& policy, Rest&&... rest)
          : Policy(std::forward<Policy>(policy)), RestBase(std::forward<Rest>(rest)...)
      {
      }
    };

    template <typename... Policies>
    struct serializer : serializer_base<Policies...>
    {
      serializer(Policies&&... policies)
          : serializer_base<Policies...>(std::forward<Policies>(policies)...)
      {
      }

      template <typename T>
      void escape(const T& t)
      {
        escape(*this, t);
      }

      using serializer_base<Policies...>::escape;
    };
  }

  template <typename... Policies>
  inline auto build_serializer(Policies&&... policies) -> serializer_impl::serializer<Policies...>
  {
    static_assert(sizeof...(Policies) > 0, "Number of policies has to be more than 0");
    return serializer_impl::serializer<Policies...>(std::forward<Policies>(policies)...);
  }
}

#endif
