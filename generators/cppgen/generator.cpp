#include <iostream>
#include <fstream>

#include "utils/contract.h"

#include "typesystem/type.h"

#include "parser/metaparser.h"
#include "parser/function.h"
#include "parser/vardecl.h"

#include "cppwriter.h"
#include "generator.h"

namespace meta {
namespace generators{
namespace cppgen {

class CppGen: public Generator {
public:
    CppGen(const Dictionary& dict): mDict(dict) {}

    void generate(meta::AST*, const std::string& output) override {
        std::ofstream out;
        out.exceptions(std::fstream::badbit);
        out.open(output, std::fstream::trunc | std::fstream::binary);
        CppWriter writer(out);

        for (const auto& kv: mDict) {
            const auto& functions = kv.second;

            writer.setPackage(kv.first);
            for (const auto& func: functions) {
                Function* f = func.second;
                if (f->visibility() != Visibility::Export)
                    continue;
                writer.forwardDeclare(f);
            }
        }
    }

private:
    const Dictionary& mDict;
};

std::unique_ptr<Generator> createCppGenerator(const Dictionary& dict) {
    return std::make_unique<CppGen>(dict);
}

}}} //namespace meta::generators::cppgen
