#include <cassert>

#include "parser/function.h"
#include "parser/package.h"

namespace meta {

Package::Package(const StackFrame *start, size_t size): Node(start, size)
{
    assert(size == 4);
    mName = start[1].tokens;
}

std::vector<Function*> Package::functions()
{
    return getChildren<Function>();
}

}
