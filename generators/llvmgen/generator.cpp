#include "parser/metanodes.h"

#include "generators/translationrunner.h"

#include "generators/llvmgen/generator.h"
#include "generators/llvmgen/modulebuilder.h"

namespace generators {
namespace llvmgen {

class LlvmGen: public generators::Generator
{
public:
    virtual void generate(meta::AST *ast, const std::string &output) override
    {
        auto pos = output.rfind('/');
        if (pos == std::string::npos)
            pos = 0;
        Environment env(output.substr(pos)); /// @todo strip extension as well
        ModuleBuilder builder(env);
        generators::TranslationRunner<llvm::Value *> runner;
        runner.translate(ast, &builder);
        builder.save(output);
    }
};

generators::Generator *createLlvmGenerator()
{
    return new LlvmGen;
}

} // namespace llvmgen
} // namespace generator
