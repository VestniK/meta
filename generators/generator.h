#ifndef GENERATORS_GENERATOR_H
#define GENERATORS_GENERATOR_H

#include <memory>
#include <string>

namespace meta {
class Package;
}

namespace generators {

class Generator
{
public:
    virtual ~Generator() {}

    virtual void generate(std::shared_ptr<meta::Package> pkg, const std::string &output) = 0;

protected:
    Generator() {}
};

} // namespace generators

#endif // GENERATORS_GENERATOR_H
