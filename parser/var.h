#ifndef VAR_H
#define VAR_H

#include <string>

#include "parser/metaparser.h"

namespace meta {

class Var: public Node
{
meta_NODE
public:
    Var(AST *ast, const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}
    meta::VarDecl *declaration() {return mDeclaration;}
    void setDeclaration(meta::VarDecl *decl) {mDeclaration = decl;}

private:
    std::string mName;
    meta::VarDecl *mDeclaration;
};

}

#endif // VAR_H
