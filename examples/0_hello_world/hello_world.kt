%namespace test
%{
	$class Hello

	%auto render() const -> void
	%{
		Hello ${data}!
	%}

	$endclass
%}
