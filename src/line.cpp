#include "line.h"
#include "parse_context.h"

namespace kiste
{
  auto line_t::update(const parse_context& ctx) -> void
  {
    curly_level = ctx.curly_level;
    trailing_return = ctx.has_trailing_return;
  }
}
