#include <iostream>
#include <sample.h>
#include <vector>
#include <kiste/raw.h>

struct Data
{
	std::vector<int> items = {1, 2, 3, 4, 5, 6};
};

int main()
{
	const auto data = Data{};
	auto& os = std::cout;
	const auto serializer = kiste::raw{os};
	auto sample = test::Sample(data, os, serializer);

	sample.render();
}
