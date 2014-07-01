#include <cassert>

#include "parser/arg.h"

namespace meta {

Arg::Arg(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 2);
    mType.assign(start[0].start, start[0].end - start[0].start);
    mName.assign(start[1].start, start[1].end - start[1].start);
}

} // namespace meta
