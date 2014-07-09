#include <cassert>

#include "parser/assigment.h"

namespace meta {

Assigment::Assigment(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 4);
    mModifiedVar.assign(start[0].start, start[0].end - start[0].start);
}

}
