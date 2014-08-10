#include <cassert>

#include "parser/vardecl.h"

namespace meta {

VarDecl::VarDecl(AST *ast, const StackFrame* start, size_t size): Node(ast, start, size), mFlags(0)
{
    assert(size == 3);
    mType = start[0].tokens;
    mName = start[1].tokens;
    mInited = !start[2].tokens.empty(); // start[2] is optional initial assigment section "['=' Expr]"
}

bool VarDecl::is(VarDecl::Flags flag) const
{
    return (mFlags & flag) != 0;
}

void VarDecl::set(VarDecl::Flags flag, bool val)
{
    mFlags = val ? (mFlags | flag) : (mFlags & ~flag);
}

}
