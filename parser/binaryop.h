#ifndef BINARY_OP_H
#define BINARY_OP_H

#include "parser/metaparser.h"

namespace meta {

class BinaryOp: public Node
{
meta_NODE
public:
    BinaryOp(const StackFrame *start, size_t size);

    enum Operation {add, sub, mul, div};
    Operation operation() const {return mOp;}

private:
    Operation mOp;
};

} // namespace meta

#endif // BINARY_OP_H
