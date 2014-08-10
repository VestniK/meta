#include <cassert>

#include "parser/function.h"
#include "parser/vardecl.h"

namespace meta {

Function::Function(AST *ast, const StackFrame* start, size_t size): Node(ast, start, size)
{
    static const size_t typePos = 0;
    static const size_t namePos = 1;
    assert(size > namePos + 1);
    mRetType = start[typePos].tokens;
    mName = start[namePos].tokens;
}

std::vector<VarDecl*> Function::args()
{
    return getChildren<VarDecl>();
}

} // namespace meta

