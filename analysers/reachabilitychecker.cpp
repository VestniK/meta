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
 *
 */

#include "parser/codeblock.h"
#include "parser/exprstatement.h"
#include "parser/if.h"
#include "parser/return.h"
#include "parser/vardecl.h"

#include "analysers/reachabilitychecker.h"
#include "analysers/semanticerror.h"

namespace analysers {

class ReachabilityChecker : public meta::Visitor
{
public:
    virtual bool visit(meta::Function *node) override;
    virtual bool visit(meta::CodeBlock *node) override;
    virtual bool visit(meta::VarDecl *node) override;
    virtual bool visit(meta::ExprStatement *node) override;
    virtual bool visit(meta::Return *node) override;
    virtual bool visit(meta::If *node) override;

private:
    void checkReturn(meta::Node *node);

private:
    meta::Return *mReturn = nullptr;
};

bool ReachabilityChecker::visit(meta::Function *)
{
    mReturn = nullptr;
    return true;
}

void ReachabilityChecker::checkReturn(meta::Node *node)
{
    if (mReturn != nullptr)
        throw SemanticError(
            node , "Code is unreachable due to return statement at position %d:%d",
            mReturn->tokens().linenum(), mReturn->tokens().colnum()
        );
}

bool ReachabilityChecker::visit(meta::CodeBlock *node)
{
    checkReturn(node);
    return true;
}

bool ReachabilityChecker::visit(meta::VarDecl *node)
{
    if (node->is(meta::VarDecl::argument))
        return false;
    checkReturn(node);
    return false;
}

bool ReachabilityChecker::visit(meta::ExprStatement *node)
{
    checkReturn(node);
    return false;
}

bool ReachabilityChecker::visit(meta::Return *node)
{
    checkReturn(node);
    mReturn = node;
    return false;
}

bool ReachabilityChecker::visit(meta::If *node)
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

void checkReachability(meta::AST *ast)
{
    ReachabilityChecker checker;
    ast->walk(&checker);
}

} // namespace analysers
