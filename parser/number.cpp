#include <cassert>
#include <cstdlib>
#include <string>

#include "parser/number.h"

namespace meta {

Number::Number(AST *ast, const StackFrame* start, size_t size): Node(ast, start, size)
{
    assert(size == 1);
    mValue = atoi(std::string(start[0].tokens).c_str());
}

}
