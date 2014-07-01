#include <cassert>

#include "parser/call.h"

namespace meta {

Call::Call(const StackFrame* reduction, size_t size): Node(reduction, size)
{
    assert(size == 4);
    mFunctionName.assign(reduction[0].start, reduction[0].end - reduction[0].start);
}

}
