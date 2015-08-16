%namespace test
%{
  $class Parent

  %auto title() const -> void
  %{
    $|Parent$|
  %}

  %auto render() const -> void
  %{
    This is the parent's header.

    Let's pull in the child's body here
    $call{child.body()}

    And here comes the parent's footer.
  %}

  $endclass
%}
