%#include <child.h>

%namespace test
%{
  $class GrandChild : Child

  %auto body() const -> void
  %{
    Grand child's body.

    We can call anything inherited here, e.g.

    Inherited title: '$call{parent.title()}'

  %}

  $endclass
%}
