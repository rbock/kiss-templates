#include <iostream>
#include <sample.h>
#include <kiste/html.h>
#include <vector>

struct Data
{
	struct Parameter
	{
		std::string type;
		std::string name;
		std::string value;
	};

	std::string documentTitle = "Less <, Greater >, And &, Quot \", Tick '";
	std::string backgroundColor = "#ddd";
	std::string formUrl = "https://www.test.com/?kiste=easy&cheesecake=yammi";
	std::string formTarget = "somewhere";
	std::vector<Parameter> params =
	{
		Parameter{"HIDDEN", "SECRET", "Shhhh!"},
		Parameter{"TEXT", "name", ""},
		Parameter{"SUBMIT", "submit", "Submit"}
	};
};

int main()
{
	const auto data = Data{};
	auto& os = std::cout;
	const auto serializer = kiste::html{os};
	auto sample = test::Sample(data, os, serializer);

	sample.render();
}
