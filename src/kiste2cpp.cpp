//#include <ciso646>  // Make MSCV understand and/or/not
#include <iostream>

namespace
{
  struct parse_error
  {
    std::string line;
  };
}

auto main(int argc, char** argv) -> int
{
  try
  {
  }
  catch (const parse_error& e)
  {
    std::cerr << "Line: " << e.line << std::endl;
    return 1;
  }
}
