#include <cassert>
#include <stdexcept>
#include <vector>

#include "fs/io.h"

#include "parser/metanodes.h"
#include "parser/package.h"

class PackageCompiler: public meta::ParseActions, public meta::NodeActions
{
public:
    PackageCompiler(Package &package): package(package) {}

    virtual void onPackage(const meta::StackFrame *start, size_t size);
    virtual void onFunction(std::shared_ptr<meta::Function> node);

private:
    Package &package;
};

void PackageCompiler::onPackage(const meta::StackFrame *start, size_t size)
{
    assert(size == 3);
    package.name.assign(start[1].start, start[1].end - start[1].start);
}

void PackageCompiler::onFunction(std::shared_ptr<meta::Function> node)
{
    if (package.functions.find(node->name()) != package.functions.end())
        throw std::runtime_error("Multiple definition of the function: " + node->name());
    package.functions[node->name()] = node;
}

void Package::parse(const std::string &sourcePath)
{
    std::vector<char> content;
    readWholeFile(sourcePath, content);
    meta::Parser parser;
    PackageCompiler compiler(*this);
    parser.setParseActions(&compiler);
    parser.setNodeActions(&compiler);
    parser.parse(content.data());
}
