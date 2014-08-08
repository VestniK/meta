#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>

#include "parser/metaparser.h"

namespace meta {

class Function: public Node
{
meta_NODE
public:
    Function(AST &ast, const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}
    const std::string &retType() const {return mRetType;}
    std::vector<Arg*> args();
private:
    std::string mName;
    std::string mRetType;
};

} // namespace meta

#endif // FUNCTION_H

