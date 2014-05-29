#ifndef META_NODES_H
#define META_NODES_H

#include <string>

#include "parser/metaparser.h"

namespace meta {

class Function: public Node
{
meta_NODE
public:
    Function(const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}
private:
    std::string mName;
};

} // namespace meta

#endif // META_NODES_H
