#include <ciso646>  // Make MSCV understand and/or/not
#include <iostream>
#include <string>

auto main() -> int
{
  try
  {
  }
  catch (const std::string& e)
  {
    std::cerr << e << std::endl;
    return 1;
  }
}
