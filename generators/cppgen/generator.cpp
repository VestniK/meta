#include <fstream>

#include "typesystem/type.h"

#include "parser/metaparser.h"
#include "parser/function.h"
#include "parser/vardecl.h"

#include "generator.h"

namespace meta {
namespace generators{
namespace cppgen {

void printFuncDecl(std::ostream& out, Function* func)
{
    out << func->type()->name() << " " << func->name() << "(";
    bool first = true;
    for (auto* arg: func->args()) {
        if (!first)
            out << ",";
        else
            first = false;
        out << arg->type()->name() << " " << arg->name();
    }
    out << ");\n";
}

class CppGen: public Generator
{
public:
    void generate(meta::AST* ast, const std::string& output) override
    {
        std::ofstream out;
        out.exceptions(std::fstream::badbit);
        out.open(output, std::fstream::trunc | std::fstream::binary);
        walk<Function, TopDown>(ast, [&out](Function* func) {
            printFuncDecl(out, func);
            return false;
        });
    }
};

std::unique_ptr<Generator> createCppGenerator()
{
    return std::make_unique<CppGen>();
}

}}} //namespace meta::generators::cppgen
