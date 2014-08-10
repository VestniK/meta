#include "parser/call.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/package.h"

#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

namespace analysers {

void resolve(meta::AST *ast)
{
    std::vector<meta::Function*> functions;
    for (auto pkg : ast->getChildren<meta::Package>()) {
        auto pkgFuncs = pkg->functions();
        functions.insert(functions.end(), pkgFuncs.begin(), pkgFuncs.end());
    }
    ast->walkTopDown<meta::Call>([&functions](meta::Call *call) {
        for (const auto func : functions) {
            if (call->functionName() != func->name())
                continue;
            call->setFunction(func);
            break;
        }
        if (call->function() == nullptr)
            throw SemanticError(call, std::string("Unresolved function call '") + call->functionName() + "'");
        auto expectedArgs = call->function()->args();
        auto passedArgs = call->getChildren<meta::Node>(1);
        if (expectedArgs.size() != passedArgs.size())
            throw SemanticError(call, std::string("Call to function '") + call->functionName() + "' with incorrect number of arguments");
        /// @todo check types
    });
}

} // namespace analysers
