#include <iostream>
#include <grand_child.h>
#include <vector>
#include <kiste/raw.h>

struct Data
{
};

int main()
{
	const auto data = Data{};
	auto& os = std::cout;
	auto serializer = kiste::raw{os};
	auto sample = test::GrandChild(data, serializer);

	sample.render();
}
