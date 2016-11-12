#pragma once

#include "parser/metanodes.h"

#include "generators/llvmgen/generator.h"
#include "generators/llvmgen/modulebuilder.h"

namespace meta {
namespace generators {
namespace llvmgen {

class LlvmGen: public Generator
{
public:
    void generate(AST* ast, const utils::fs::path& output) override {
        Environment env(output.filename().string()); /// @todo strip extension as well
        ModuleBuilder builder(env);
        ast->walk(&builder);
        builder.save(output);
    }
};

std::unique_ptr<Generator> createLlvmGenerator()
{
    return std::make_unique<LlvmGen>();
}

} // namespace llvmgen
} // namespace generator
} // namespace meta
