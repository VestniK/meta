#include <cassert>

#include "parser/vardecl.h"

namespace meta {

VarDecl::VarDecl(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 4);
    mType = start[0].tokens;
    mName = start[1].tokens;
    mInited = !start[2].tokens.empty(); // start[2] is optional initial assigment section "['=' Expr]"
}

}
