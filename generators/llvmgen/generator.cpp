#include "parser/metanodes.h"

#include "generators/llvmgen/generator.h"
#include "generators/llvmgen/modulebuilder.h"

namespace generators {
namespace llvmgen {

class LlvmGen: public generators::Generator
{
public:
    virtual void generate(std::shared_ptr<meta::Package> pkg, const std::string& output) override
    {
        Environment env(pkg->name());
        for (auto func : pkg->functions())
            env.addFunction(func);
        ModuleBuilder builder(env);
        pkg->walk(&builder);
        builder.save(output);
    }
};

generators::Generator *createLlvmGenerator()
{
    return new LlvmGen;
}

} // namespace llvmgen
} // namespace generator
