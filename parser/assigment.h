#ifndef ASSIGMENT_H
#define ASSIGMENT_H

#include "parser/metaparser.h"

namespace meta {

class Assigment: public Node
{
meta_NODE
public:
    Assigment(AST *ast, const StackFrame* start, size_t size);

    const std::string &varName() const {return mVarName;}
    VarDecl *declaration() {return mDeclaration;}
    void setDeclaration(VarDecl *decl) {mDeclaration = decl;}

private:
    std::string mVarName;
    VarDecl *mDeclaration;
};

}

#endif // ASSIGMENT_H
