#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>

#include "parser/metaparser.h"

namespace meta {

class Function: public Node
{
meta_NODE
public:
    Function(const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}
    const std::string &retType() const {return mRetType;}
private:
    std::string mName;
    std::string mRetType;
};

} // namespace meta

#endif // FUNCTION_H

