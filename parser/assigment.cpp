#include <cassert>

#include "parser/assigment.h"

namespace meta {

Assigment::Assigment(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 3);
    mVarName = start[0].tokens;
}

}
