#include <cassert>

#include "parser/vardecl.h"

namespace meta {

VarDecl::VarDecl(AST *ast, const StackFrame* start, size_t size): Node(ast, start, size)
{
    assert(size == 3);
    mType = start[0].tokens;
    mName = start[1].tokens;
    mInited = !start[2].tokens.empty(); // start[2] is optional initial assigment section "['=' Expr]"
}

}
