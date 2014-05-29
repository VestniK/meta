#include <cassert>

#include "metanodes.h"

namespace meta {

Function::Function(const meta::StackFrame* start, size_t size): Node(start, size)
{
    assert(size > 1);
    static const size_t namePos = 0;
    mName.assign(start[namePos].start, start[namePos].end - start[namePos].start);
}

} // namespace meta
