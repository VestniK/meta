#include <cassert>

#include "astutils/childrengatherer.h"

#include "parser/function.h"
#include "parser/package.h"

namespace meta {

Package::Package(const StackFrame *start, size_t size): Node(start, size)
{
    assert(size == 4);
    mName.assign(start[1].start, start[1].end - start[1].start);
}

std::vector<Function*> Package::functions()
{
    astutils::ChildrenGatherer<Function> funcGatherer;
    walk(&funcGatherer);
    return funcGatherer.gathered();
}

}
