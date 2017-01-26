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

#include <complex>
#include <iostream>
#include <kiste/html.h>
#include <kiste/serializer_builder.h>
#include <sample.h>

struct ratio
{
  int num;
  int den;
};

struct html : kiste::html
{
  html(std::ostream& os) : kiste::html(os)
  {
  }

  template <typename SerializerT, typename T>
  void escape(SerializerT&, const T& t)
  {
    kiste::html::escape(t);
  }
};

struct ratio_policy
{
  template <typename SerializerT>
  void escape(SerializerT& serializer, const ratio& value)
  {
    serializer.escape(value.num);
    if (value.den != 1)
    {
      serializer.escape('/');
      serializer.escape(value.den);
    }
  }
};

struct complex_policy
{
  template <typename SerializerT, typename T>
  void escape(SerializerT& serializer, const std::complex<T>& value)
  {
    serializer.escape(value.real());
    serializer.escape(" + ");
    serializer.escape(value.imag());
    serializer.escape(" * i");
  }
};

struct Data
{
  ratio one = {1, 1};
  ratio one_percent = {1, 100};
  std::complex<double> complex_value_with_double = {1. / 3., 3. / 4.};
  std::complex<ratio> complex_value_with_ratio = {ratio{1, 3}, ratio{3, 4}};
};

int main()
{
  const auto data = Data{};
  auto& os = std::cout;
  auto serializer = kiste::build_serializer(html{os}, complex_policy{}, ratio_policy{});
  auto sample = test::Sample(data, serializer);

  sample.render();
}
