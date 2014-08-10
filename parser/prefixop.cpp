#include <cassert>

#include "parser/prefixop.h"

namespace meta {

PrefixOp::PrefixOp(AST *ast, const StackFrame *start, size_t size): Node(ast, start, size)
{
    assert(size == 2);
    switch (*start[0].tokens.begin()->start) {
        case '-': mOperation = negative; break;
        case '+': mOperation = positive; break;
    }
}

} // namespace meta
