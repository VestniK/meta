#include <cassert>

#include "parser/var.h"

namespace meta {

Var::Var(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 1);
    mName = start[0].tokens;
}

} // namespace meta