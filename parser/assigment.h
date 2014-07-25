#ifndef ASSIGMENT_H
#define ASSIGMENT_H

#include "parser/metaparser.h"

namespace meta {

class Assigment: public Node
{
meta_NODE
public:
    Assigment(const StackFrame* start, size_t size);

    const std::string &varName() const {return mVarName;}

private:
    std::string mVarName;
};

}

#endif // ASSIGMENT_H
