%#include <ctime>

%namespace test
%{
	$class Sample

	%auto render() const -> void
	%{
		We print this at ${std::time(nullptr)}:
		<ul>
			%for (auto&& item : data.items)
			%{
				<li>${item}</li>
			%}
		<ul>
	%}

	$endclass
%}
