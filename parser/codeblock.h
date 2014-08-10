#ifndef CODEBLOCK_H
#define CODEBLOCK_H

#include "parser/metaparser.h"

namespace meta {

class CodeBlock: public Node
{
meta_NODE
public:
    CodeBlock(AST *ast, const StackFrame *start, size_t size);
};

} // namespace meta

#endif // CODEBLOCK_H
