#include <cassert>

#include "parser/function.h"
#include "parser/vardecl.h"

namespace meta {

Function::Function(AST *ast, const StackFrame* start, size_t size): Node(ast, start, size)
{
    static const size_t typePos = 0;
    static const size_t namePos = 1;
    static const size_t argsPos = 3;
    assert(size > argsPos + 1);
    mRetType = start[typePos].tokens;
    mName = start[namePos].tokens;
    for (auto arg : start[argsPos].nodes)
        arg->walkTopDown<meta::VarDecl>([] (meta::VarDecl *node) {node->set(meta::VarDecl::argument);}, 1);
}

std::vector<VarDecl*> Function::args()
{
    return getChildren<VarDecl>();
}

} // namespace meta

