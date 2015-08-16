#include <iostream>
#include <sample.h>
#include <kiste/raw.h>

struct Data
{
};

int main()
{
	const auto data = Data{};
	auto& os = std::cout;
	const auto serializer = kiste::raw{os};
	auto sample = test::Sample(data, os, serializer);

	sample.render();
}
