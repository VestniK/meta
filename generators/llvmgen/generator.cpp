#include "parser/metanodes.h"

#include "generators/llvmgen/generator.h"
#include "generators/llvmgen/modulebuilder.h"

namespace meta {
namespace generators {
namespace llvmgen {

class LlvmGen: public Generator
{
public:
    virtual void generate(AST *ast, const std::string &output) override
    {
        auto pos = output.rfind('/');
        if (pos == std::string::npos)
            pos = 0;
        Environment env(output.substr(pos)); /// @todo strip extension as well
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
