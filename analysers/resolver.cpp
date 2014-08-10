#include <map>

#include "parser/assigment.h"
#include "parser/call.h"
#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/package.h"
#include "parser/var.h"
#include "parser/vardecl.h"

#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

namespace analysers {

class ResolveVisitor: public meta::Visitor
{
public:
    ResolveVisitor(meta::AST *ast)
    {
        for (auto pkg : ast->getChildren<meta::Package>()) {
            auto pkgFuncs = pkg->functions();
            mFunctions.insert(mFunctions.end(), pkgFuncs.begin(), pkgFuncs.end());
        }
    }

    virtual void visit(meta::Call *node) override
    {
        for (const auto func : mFunctions) {
            if (node->functionName() != func->name())
                continue;
            node->setFunction(func);
            break;
        }
        if (node->function() == nullptr)
            throw SemanticError(node, std::string("Unresolved function call '") + node->functionName() + "'");
        auto expectedArgs = node->function()->args();
        auto passedArgs = node->getChildren<meta::Node>(1);
        if (expectedArgs.size() != passedArgs.size())
            throw SemanticError(node, std::string("Call to function '") + node->functionName() + "' with incorrect number of arguments");
        /// @todo check types
    }

    virtual void visit(meta::Function *) override
    {
        mVars.clear();
    }

    virtual void visit(meta::VarDecl *node) override
    {
        auto prev = mVars.find(node->name());
        if (prev != mVars.end())
            throw SemanticError(node, std::string("Redefinition of the variable '") + node->name() + "'");
        mVars[node->name()] = node;
    }

    virtual void visit(meta::Var *node) override
    {
        auto decl = mVars.find(node->name());
        if (decl == mVars.end())
            throw SemanticError(node, std::string("Reference to undefined variable '") + node->name() + "'");
        node->setDeclaration(decl->second);
    }

    virtual void visit(meta::Assigment *node) override
    {
        auto decl = mVars.find(node->varName());
        if (decl == mVars.end())
            throw SemanticError(node, std::string("Reference to undefined variable '") + node->varName() + "'");
        if (decl->second->is(meta::VarDecl::argument))
            throw SemanticError(node, std::string("Attempt to modify function argument '") + node->varName() + "'");
        node->setDeclaration(decl->second);
    }

private:
    std::vector<meta::Function*> mFunctions;
    std::map<std::string, meta::VarDecl*> mVars;
};

void resolve(meta::AST *ast)
{
    ResolveVisitor resolver(ast);
    ast->walk(&resolver);
}

} // namespace analysers
