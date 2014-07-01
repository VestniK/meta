#ifndef VAR_H
#define VAR_H

#include <string>

#include "parser/metaparser.h"

namespace meta {

class Var: public Node
{
meta_NODE
public:
    Var(const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}

private:
    std::string mName;
};

}

#endif // VAR_H
