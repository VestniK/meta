#ifndef PREFIXOP_H
#define PREFIXOP_H

#include "parser/metaparser.h"

namespace meta {

class PrefixOp: public Node
{
meta_NODE
public:
    PrefixOp(AST &ast, const StackFrame *start, size_t size);

    enum Operation {negative, positive};
    Operation operation() const {return mOperation;}

private:
    Operation mOperation;
};

} // namespace meta

#endif // PREFIXOP_H
