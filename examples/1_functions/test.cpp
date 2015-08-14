#include <iostream>
#include <functions.h>

struct Data
{
	float foo;
};

int main()
{
	const auto data = Data{3.1415};
	auto& os = std::cout;
	const auto serializer = kiste::raw{os};
	auto sample = test::Sample(data, os, serializer);

	sample.render();
}
