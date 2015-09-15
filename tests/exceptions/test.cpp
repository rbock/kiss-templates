#include <ciso646>  // Make MSCV understand and/or/not
#include <iostream>
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
