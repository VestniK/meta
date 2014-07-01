#include <cassert>
#include <cstdlib>
#include <string>

#include "parser/number.h"

namespace meta {

Number::Number(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 1);
    std::string strVal(start->start, start->end - start->start);
    mValue = atoi(strVal.c_str());
}

}
