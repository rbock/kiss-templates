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

#include <ciso646>  // Make MSCV understand and/or/not
#include <iostream>
#include <string>
#include <sample.h>
#include <kiste/raw.h>
#include <vector>
#include <exception>

struct Data
{
};

struct Serializer : public kiste::raw
{
  bool caughtInt = false;
  bool caughtString = false;
  bool caughtException = false;

  Serializer(std::ostream& os) : kiste::raw(os)
  {
  }

  void report_exception(long lineNo, const std::string& expression, std::exception_ptr e)
  {
    try
    {
      std::rethrow_exception(e);
    }
    catch (const int& e)
    {
      text("Caught an int '");
      escape(e);
      text("'");
      caughtInt = true;
    }
    catch (const std::string& e)
    {
      text("Caught a std::string '");
      escape(e);
      text("'");
      caughtString = true;
    }
    catch (const std::exception& e)
    {
      text("Caught a std::exception '");
      escape(e.what());
      text("'");
      caughtException = true;
    }
    catch (...)
    {
      text("Caught an unknown exception");
    }
    text(" at line ");
    escape(lineNo);
    text(" in expression(");
    escape(expression);
    text(")");
  }
};

int main()
{
  const auto data = Data{};
  auto& os = std::cout;
  auto serializer = Serializer{os};
  auto sample = test::Sample(data, serializer);

  sample.render();
  if (not(serializer.caughtInt and serializer.caughtString and serializer.caughtException))
  {
    std::cerr << "Missed some expected exception! Please inspect" << std::endl;
    return 1;
  }
}
