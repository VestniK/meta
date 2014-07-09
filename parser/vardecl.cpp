#include <cassert>

#include "parser/vardecl.h"

namespace meta {

VarDecl::VarDecl(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 4);
    mType.assign(start[0].start, start[0].end - start[0].start);
    mName.assign(start[1].start, start[1].end - start[1].start);
    mInited = start[2].end != start[2].start; // start[2] is optional initial assigment section "['=' Expr]"
}

}
