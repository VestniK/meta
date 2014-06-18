#include <cassert>
#include <cstdlib>

#include "metanodes.h"

namespace meta {

Function::Function(const meta::StackFrame* start, size_t size): Node(start, size)
{
    assert(size > 1);
    static const size_t namePos = 0;
    mName.assign(start[namePos].start, start[namePos].end - start[namePos].start);
}

BinaryOp::BinaryOp(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 3);
    static const size_t opPos = 1;
    switch (*start[opPos].start) {
        case '+': mOp = add; break;
        case '-': mOp = sub; break;
        case '*': mOp = mul; break;
        case '/': mOp = div; break;
    }
}

Number::Number(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 1);
    std::string strVal(start->start, start->end - start->start);
    mValue = atoi(strVal.c_str());
}

Var::Var(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 1);
    mName.assign(start->start, start->end - start->start);
}

} // namespace meta
