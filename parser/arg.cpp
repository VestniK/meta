#include <cassert>

#include "parser/arg.h"

namespace meta {

Arg::Arg(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 2);
    mType = start[0].tokens;
    mName = start[1].tokens;
}

} // namespace meta
