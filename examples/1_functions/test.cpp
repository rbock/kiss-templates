#include <iostream>
#include <functions.h>
#include <kiste/raw.h>

struct Data
{
	double foo;
};

int main()
{
	const auto data = Data{3.1415};
	auto& os = std::cout;
	auto serializer = kiste::raw{os};
	auto sample = test::Sample(data, serializer);

	sample.render();
}
