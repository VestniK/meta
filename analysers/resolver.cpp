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
            throw SemanticError(node, "Unresolved function call '%s'", node->functionName().c_str());
        auto expectedArgs = node->function()->args();
        auto passedArgs = node->getChildren<meta::Node>(1);
        if (expectedArgs.size() != passedArgs.size())
            throw SemanticError(node, "Call to function '%s' with incorrect number of arguments", node->functionName().c_str());
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
            throw SemanticError(
                node, "Redefinition of the variable '%s' first defined at line %d, column %d",
                node->name().c_str(), prev->second->tokens().begin()->line, prev->second->tokens().begin()->column
            );
        mVars[node->name()] = node;
    }

    virtual void visit(meta::Var *node) override
    {
        auto decl = mVars.find(node->name());
        if (decl == mVars.end())
            throw SemanticError(node, "Reference to undefined variable '%s'", node->name().c_str());
        node->setDeclaration(decl->second);
    }

    virtual void visit(meta::Assigment *node) override
    {
        auto decl = mVars.find(node->varName());
        if (decl == mVars.end())
            throw SemanticError(node, "Reference to undefined variable '%s'", node->varName().c_str());
        if (decl->second->is(meta::VarDecl::argument))
            throw SemanticError(node, "Attempt to modify function argument '%s'", node->varName().c_str());
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
