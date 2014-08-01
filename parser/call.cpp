#include <cassert>

#include "parser/call.h"

namespace meta {

Call::Call(const StackFrame* reduction, size_t size): Node(reduction, size)
{
    assert(size == 4);
    mFunctionName = reduction[0].tokens;
}

}
