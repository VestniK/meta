#include <cassert>

#include "parser/arg.h"
#include "parser/function.h"

namespace meta {

Function::Function(const meta::StackFrame* start, size_t size): Node(start, size)
{
    static const size_t typePos = 0;
    static const size_t namePos = 1;
    assert(size > namePos + 1);
    mRetType.assign(start[typePos].start, start[typePos].end - start[typePos].start);
    mName.assign(start[namePos].start, start[namePos].end - start[namePos].start);
}

std::vector<Arg*> Function::args()
{
    return getChildren<Arg>();
}

} // namespace meta

