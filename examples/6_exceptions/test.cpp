#include <iostream>
#include <sample.h>
#include <kiste/raw.h>
#include <vector>
#include <exception>

struct Data
{
};

struct Serializer: public kiste::raw
{
  bool caughtInt = false;
  bool caughtString = false;
  bool caughtException = false;

  using raw::raw;

  void report_exception(long lineNo, const std::string& expression, std::exception_ptr e)
  {
    try
    {
      std::rethrow_exception(e);
    }
    catch(const int& e)
    {
      _os << "Caught an int '" << e << "'";
      caughtInt = true;
    }
    catch(const std::string& e)
    {
      _os << "Caught a std::string '" << e << "'";
      caughtString = true;
    }
    catch(const std::exception& e)
    {
      _os << "Caught a std::exception '" << e.what() << "'";
      caughtException = true;
    }
    catch(...)
    {
      _os << "Caught an unknown exception";
    }
    _os << " at line " << lineNo << " in expression(" << expression << ")";
  }

};

int main()
{
  const auto data = Data{};
  auto& os = std::cout;
  auto serializer = Serializer{os};
  auto sample = test::Sample(data, serializer);

  sample.render();
  if (not (serializer.caughtInt and serializer.caughtString and serializer.caughtException))
  {
    std::cerr << "Missed some expected exception! Please inspect" << std::endl;
    return 1;
  }
}
