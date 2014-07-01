#ifndef NUMBER_H
#define NUMBER_H

#include "parser/metaparser.h"

namespace meta {

class Number: public Node
{
meta_NODE
public:
    Number(const StackFrame *start, size_t size);

    int value() const {return mValue;}

private:
    int mValue;
};

}

#endif // NUMBER_H
