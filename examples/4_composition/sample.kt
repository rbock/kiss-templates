%#include <helper.h>

%namespace test
%{
  $class Sample
  $member Helper helper

  %auto render() const -> void
  %{
    Helper's answer: ${helper.getAnswer()}.

    Helper's rendering: $call{helper.render()}
  %}

  $endclass
%}
