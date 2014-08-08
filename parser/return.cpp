#include "parser/return.h"

namespace meta {

Return::Return(AST &ast, const StackFrame* reduction, size_t size): Node(ast, reduction, size)
{
}

}
