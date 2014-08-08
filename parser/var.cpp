#include <cassert>

#include "parser/var.h"

namespace meta {

Var::Var(AST &ast, const StackFrame* start, size_t size): Node(ast, start, size)
{
    assert(size == 1);
    mName = start[0].tokens;
}

} // namespace meta