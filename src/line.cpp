#include "line.h"
#include "parse_context.h"

namespace kiste
{
  auto line_t::update(const parse_context& ctx) -> void
  {
    curly_level = ctx._curly_level;
    trailing_return = ctx._has_trailing_return;
  }
}
