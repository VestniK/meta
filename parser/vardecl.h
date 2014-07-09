#ifndef VAR_DECL_H
#define VAR_DECL_H

#include <string>

#include "parser/metaparser.h"

namespace meta {

class VarDecl: public meta::Node
{
meta_NODE
public:
    VarDecl(const StackFrame* start, size_t size);

    const std::string &name() const {return mName;}
    const std::string &type() const {return mType;}
    bool inited() const {return mInited;}

private:
    std::string mName, mType;
    bool mInited;
};

}

#endif // VAR_DECL_H
