#ifndef META_NODES_H
#define META_NODES_H

#include "parser/metaparser.h"

namespace meta {

class Function: public Node
{
meta_NODE
public:
    Function(StackFrame *start, int size);
};

} // namespace meta

#endif // META_NODES_H
