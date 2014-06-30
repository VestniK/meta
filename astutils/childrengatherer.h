#ifndef CHILDREN_GATHERER_H
#define CHILDREN_GATHERER_H

#include <vector>

#include "parser/metaparser.h"

namespace astutils {

template<class T>
class ChildrenGatherer: public meta::Visitor
{
public:
    virtual void visit(T *node) override {mGathered.push_back(node);}

    const std::vector<T *> &gathered() const {return mGathered;}

private:
    std::vector<T *> mGathered;
};

} // namespace astutils

#endif // CHILDREN_GATHERER_H
