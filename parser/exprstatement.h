#ifndef EXPRSTATEMENT_H
#define EXPRSTATEMENT_H

#include "parser/metaparser.h"

namespace meta {

class ExprStatement: public Node
{
meta_NODE
public:
    ExprStatement(const StackFrame *start, size_t size);
};

} // namespace meta

#endif // EXPRSTATEMENT_H
