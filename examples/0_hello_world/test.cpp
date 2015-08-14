#include <iostream>
#include <hello_world.h>

int main()
{
	const auto data = "World";
	auto& os = std::cout;
	const auto serializer = kiste::raw{os};
	auto hello = test::Hello(data, os, serializer);

	hello.render();
}
