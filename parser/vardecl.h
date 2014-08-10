#ifndef VAR_DECL_H
#define VAR_DECL_H

#include <string>

#include "parser/metaparser.h"

namespace meta {

class VarDecl: public meta::Node
{
meta_NODE
public:
    VarDecl(AST *ast, const StackFrame* start, size_t size);

    enum Flags
    {
        argument = (1<<0)
    };

    const std::string &name() const {return mName;}
    const std::string &type() const {return mType;}
    bool inited() const {return mInited;}
    bool is(Flags flag) const;
    void set(Flags flag, bool val = true);

private:
    std::string mName, mType;
    int mFlags;
    bool mInited;
};

}

#endif // VAR_DECL_H
