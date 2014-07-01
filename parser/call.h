#include <string>

#include "parser/metaparser.h"

namespace meta {

class Call: public Node
{
meta_NODE
public:
    Call(const StackFrame *reduction, size_t size);

    const std::string &functionName() const {return mFunctionName;}

private:
    std::string mFunctionName;
};

}
