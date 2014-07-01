#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <vector>

#include "parser/metaparser.h"

namespace meta {

class Package: public Node
{
meta_NODE
public:
    Package(const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}
    std::vector<Function *> functions();
private:
    std::string mName;
};

}

#endif // PACKAGE_H
