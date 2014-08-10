#include <cassert>

#include "parser/call.h"

namespace meta {

Call::Call(AST &ast, const StackFrame* reduction, size_t size):
    Node(ast, reduction, size),
    mFunction(nullptr)
{
    assert(size == 4);
    mFunctionName = reduction[0].tokens;
}

}
