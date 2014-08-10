#ifndef GENERATORS_GENERATOR_H
#define GENERATORS_GENERATOR_H

#include <memory>
#include <string>

namespace meta {
class AST;
}

namespace generators {

class Generator
{
public:
    virtual ~Generator() {}

    virtual void generate(meta::AST *ast, const std::string &output) = 0;

protected:
    Generator() {}
};

} // namespace generators

#endif // GENERATORS_GENERATOR_H
