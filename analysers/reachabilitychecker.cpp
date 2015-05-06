/*
 * Meta language compiler
 * Copyright (C) 2014  Sergey Vidyuk <sir.vestnik@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "parser/codeblock.h"
#include "parser/exprstatement.h"
#include "parser/if.h"
#include "parser/function.h"
#include "parser/return.h"
#include "parser/vardecl.h"

#include "typesystem/type.h"

#include "analysers/reachabilitychecker.h"
#include "analysers/semanticerror.h"

namespace meta {
namespace analysers {

class ReachabilityChecker : public Visitor
{
public:
    virtual bool visit(Function *node) override;
    virtual void leave(Function *node) override;
    virtual bool visit(CodeBlock *node) override;
    virtual bool visit(VarDecl *node) override;
    virtual bool visit(ExprStatement *node) override;
    virtual bool visit(Return *node) override;
    virtual bool visit(If *node) override;

private:
    void checkReturn(Node *node);

private:
    Return *mReturn = nullptr;
};

bool ReachabilityChecker::visit(Function *)
{
    mReturn = nullptr;
    return true;
}

void ReachabilityChecker::leave(Function *node)
{
    /// @todo unit tests needed for non-void function without return
    if (mReturn != nullptr || node->body() == nullptr)
        return;
    if (node->type()->typeId() != typesystem::Type::Void)
        throw SemanticError(node, "Non-void function ends without return");
    node->body()->add(new Return(nullptr, 0)); /// @todo not needed any more since StatementBuilder duing the job
}


void ReachabilityChecker::checkReturn(Node *node)
{
    if (mReturn != nullptr)
        throw SemanticError(
            node , "Code is unreachable due to return statement at position %d:%d",
            mReturn->tokens().linenum(), mReturn->tokens().colnum()
        );
}

bool ReachabilityChecker::visit(CodeBlock *node)
{
    checkReturn(node);
    return true;
}

bool ReachabilityChecker::visit(VarDecl *node)
{
    if (node->is(VarDecl::argument))
        return false;
    checkReturn(node);
    return false;
}

bool ReachabilityChecker::visit(ExprStatement *node)
{
    checkReturn(node);
    return false;
}

bool ReachabilityChecker::visit(Return *node)
{
    checkReturn(node);
    mReturn = node;
    return false;
}

bool ReachabilityChecker::visit(If *node)
{
    checkReturn(node);
    if (node->thenBlock()) {
        node->thenBlock()->walk(this);
        mReturn = nullptr;
    }
    if (node->elseBlock()) {
        node->elseBlock()->walk(this);
        mReturn = nullptr;
    }
    return false;
}

void checkReachability(AST *ast)
{
    ReachabilityChecker checker;
    ast->walk(&checker);
}

} // namespace analysers
} // namespace meta

