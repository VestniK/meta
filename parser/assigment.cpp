#include <cassert>

#include "parser/assigment.h"

namespace meta {

Assigment::Assigment(AST *ast, const StackFrame* start, size_t size): Node(ast, start, size)
{
    assert(size == 3);
    mVarName = start[0].tokens;
}

}
