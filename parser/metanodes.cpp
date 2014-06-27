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

Arg::Arg(const StackFrame* start, size_t size): Node(start, size)
{
    assert(size == 2);
    mType.assign(start[0].start, start[0].end - start[0].start);
    mName.assign(start[1].start, start[1].end - start[1].start);
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

Call::Call(const StackFrame* reduction, size_t size): Node(reduction, size)
{
    assert(size == 4);
    mFunctionName.assign(reduction[0].start, reduction[0].end - reduction[0].start);
}

Return::Return(const StackFrame* reduction, size_t size): Node(reduction, size)
{
}

} // namespace meta
