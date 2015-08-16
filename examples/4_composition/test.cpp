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
	auto serializer = kiste::raw{os};
	auto sample = test::Sample(data, serializer);

	sample.render();
}
