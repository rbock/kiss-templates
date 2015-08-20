#include <iostream>
#include <calling_non_void.h>
#include <kiste/raw.h>

struct Data
{
};

int main()
{
  const auto data = Data{};
  auto& os = std::cout;
  auto serializer = kiste::raw{os};
  auto hello = assert_test::CallingNonVoid(data, serializer);

  hello.render();
}
