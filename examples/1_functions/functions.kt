%namespace test
%{
	$class Sample

	%auto render() const -> void
	%{
		You can call void functions: $call{render_bar()}.

		The serializer will eat ${"any expression"},
		for example a call to a function

		Some text function: ${yield_text()}.
		Some int function: ${yield_int()}.
		Some value: ${data.foo}.
	%}

	%auto render_bar() const -> void
	%{
		This function renders some text.
		You can call functions again ${yield_int()}!$|
	%}

	%auto yield_text() const -> std::string
	%{
		%return "Well, well, well...";
	%}

	%auto yield_int() const -> int
	%{
		%return 42;
	%}

	$endclass
%}
