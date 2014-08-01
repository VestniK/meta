#include <cassert>

#include "parser/binaryop.h"

namespace meta {

BinaryOp::BinaryOp(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 3);
    static const size_t opPos = 1;
    switch (*start[opPos].tokens.begin()->start) {
        case '+': mOp = add; break;
        case '-': mOp = sub; break;
        case '*': mOp = mul; break;
        case '/': mOp = div; break;
    }
}

} // namespace meta
