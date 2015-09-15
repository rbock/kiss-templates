#include <ciso646>  // Make MSCV understand and/or/not
#include <iostream>
#include <string>

namespace
{
  struct MyException
  {
    std::string line;
  };
}

auto main() -> int
{
  try
  {
  }
  catch (const MyException& e)
  {
    std::cerr << e.line << std::endl;
    return 1;
  }
}
