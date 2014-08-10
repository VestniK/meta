#include "parser/codeblock.h"

namespace meta {

CodeBlock::CodeBlock(AST *ast, const StackFrame *start, size_t size): Node(ast, start, size)
{
}

} // namespace meta
