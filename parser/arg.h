#ifndef ARG_H
#define ARG_H

#include <string>

#include "parser/metaparser.h"

namespace meta {

class Arg: public Node
{
meta_NODE
public:
    Arg(AST &ast, const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}
    const std::string &type() const {return mType;}

private:
    std::string mName, mType;
};

} // namespace meta

#endif // ARG_H
