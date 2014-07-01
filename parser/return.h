#ifndef RETURN_H
#define RETURN_H

#include "parser/metaparser.h"

namespace meta {

class Return: public Node
{
meta_NODE
public:
    Return(const StackFrame *reduction, size_t size);
};

}

#endif // RETURN_H
