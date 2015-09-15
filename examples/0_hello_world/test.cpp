#include <iostream>
#include <string>
#include <hello_world.h>
#include <kiste/raw.h>

struct Data
{
  std::string name;
};

int main()
{
  const auto data = Data{"World"};
  auto& os = std::cout;
  auto serializer = kiste::raw{os};
  auto hello = test::Hello(data, serializer);

  hello.render();
}
