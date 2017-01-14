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
#pragma once

#include <stack>
#include <vector>

#include "utils/contract.h"

#include "parser/metanodes.h"

#include "typesystem/type.h"

#include "analysers/semanticerror.h"

#include "scope.hpp"

namespace meta::analysers {
namespace {

struct TypeEvaluator {
    utils::optional<Type> operator() (Node* node, Scope&) {throw UnexpectedNode(node, "Can't evaluate type");}

    utils::optional<Type> operator() (Number* node, Scope& scope) {
        node->setType(scope.findType(typesystem::BuiltinType::Int));
        return node->type();
    }

    utils::optional<Type> operator() (Literal* node, Scope& scope) {
        switch (node->value()) {
        case Literal::trueVal:
        case Literal::falseVal:
            node->setType(scope.findType(typesystem::BuiltinType::Bool));
        break;
        }
        return node->type();
    }

    utils::optional<Type> operator() (StrLiteral* node, Scope& scope) {
        node->setType(scope.findType(typesystem::BuiltinType::String));
        return node->type();
    }

    utils::optional<Type> operator() (Var* node, Scope&) {
        PRECONDITION(node->declaration() != nullptr);
        PRECONDITION(node->declaration()->type());
        PRECONDITION(node->declaration()->type()->properties() & typesystem::TypeProp::complete);
        node->setType(node->declaration()->type());

        return node->type();
    }

    utils::optional<Type> operator() (Assigment* node, Scope& scope) {
        utils::optional<Type> valueType = dispatch(*this, node->value(), scope);

        struct {
            utils::optional<Type> operator() (Node* node, utils::optional<Type>) {
              throw UnexpectedNode(node, "Variable or access to Memeber required");
            }

            utils::optional<Type> operator() (Var* node, utils::optional<Type> valType) {
                if (!node->type())
                    node->setType(node->declaration()->type());
                if (node->type() != valType)
                    throw SemanticError(
                        node, "Attempt to assign value of type '%s' to a the variable '%s' of type '%s'",
                        valType->name(), node->declaration()->name(), node->declaration()->type()->name()
                    );
                return node->declaration()->type();
            }
            utils::optional<Type> operator() (MemberAccess* node, utils::optional<Type> valType) {
                if (node->memberDecl()->type() != valType)
                    throw SemanticError(
                        node, "Attempt to assign value of type '%s' to a the variable '%s.%s.%s' of type '%s'",
                        valType->name(),
                        node->targetStruct()->package(), node->targetStruct()->name(),
                        node->memberDecl()->name(),
                        node->memberDecl()->type()->name()
                    );
                return node->memberDecl()->type();
            }
        } getAssignType;
        utils::optional<Type> targetType = dispatch(getAssignType, node->target(), valueType);
        node->setType(targetType);
        return node->type();
    }

    utils::optional<Type> operator() (PrefixOp* node, Scope& scope) {
        utils::optional<Type> operandType = dispatch(*this, node->operand(), scope);
        switch (node->operation()) {
            case PrefixOp::positive:
            case PrefixOp::negative:
                if (!(operandType->properties() & typesystem::TypeProp::numeric))
                    throw SemanticError(
                        node,
                        "Can't perform arythmetic operation on value of type '%s'", operandType->name()
                    );
                break;
            case PrefixOp::boolnot:
                if (!(operandType->properties() & typesystem::TypeProp::boolean))
                    throw SemanticError(
                        node,
                        "Can't perform boolean not operation on value of type '%s'", operandType->name()
                    );
                break;
        }
        node->setType(operandType);
        return operandType;
    }

    utils::optional<Type> operator() (BinaryOp* node, Scope& scope) {
        POSTCONDITION(node->type());
        utils::optional<Type> lhs = dispatch(*this, node->left(), scope);
        utils::optional<Type> rhs = dispatch(*this, node->right(), scope);

        switch (node->operation()) {
            case BinaryOp::add:
            case BinaryOp::div:
            case BinaryOp::sub:
            case BinaryOp::mul:
                if (
                    !(lhs->properties() & typesystem::TypeProp::numeric) ||
                    !(rhs->properties() & typesystem::TypeProp::numeric)
                )
                    throw SemanticError(
                        node, "Can't perform arythmetic operation on values of types '%s' and '%s'",
                        lhs->name(), rhs->name()
                    );
                /// @todo should return widest type of left or right. Now there is only one numeric
                // type so it's ok to return first arg type
                node->setType(lhs);
                break;

            case BinaryOp::equal:
            case BinaryOp::noteq:
                if (lhs != rhs)
                    throw SemanticError(node, "Can't compare values of types '%s' and '%s'", lhs->name(), rhs->name());
                node->setType(scope.findType(typesystem::BuiltinType::Bool));
                break;
            case BinaryOp::greater:
            case BinaryOp::greatereq:
            case BinaryOp::less:
            case BinaryOp::lesseq:
                if (
                    !(lhs->properties() & typesystem::TypeProp::numeric) ||
                    !(rhs->properties() & typesystem::TypeProp::numeric)
                )
                    throw SemanticError(node, "Can't compare values of types '%s' and '%s'", lhs->name(), rhs->name());
                node->setType(scope.findType(typesystem::BuiltinType::Bool));
                break;

            case BinaryOp::boolAnd:
            case BinaryOp::boolOr:
                if (
                    !(lhs->properties() & typesystem::TypeProp::boolean) ||
                    !(rhs->properties() & typesystem::TypeProp::boolean)
                )
                    throw SemanticError(
                        node, "Can't perform boolean operations on values of types '%s' and '%s'",
                        lhs->name(), rhs->name()
                    );
                node->setType(scope.findType(typesystem::BuiltinType::Bool));
                break;
        }
        return node->type();
    }

    // Requires TypeChecker as complete type so definition is bellow
    utils::optional<Type> operator() (Call* node, Scope& scope);
};

utils::optional<Type> type_of(Node* node, Scope& scope) {
    return dispatch(TypeEvaluator{}, node, scope);
}

class TypeChecker: public Visitor {
public:
    explicit TypeChecker(Scope& scope): mScope(scope) {}

    bool visit(Function* node) override {
        if (node->type())
            return false;
        node->setType(mScope.findType(node->retType()));
        if (!node->type())
            throw SemanticError(node, "Function '%s' returns unknown type '%s'", node->name(), node->retType());
        mCurrFunc = node;
        return true;
    }
    void leave(Function* node) override {
        POSTCONDITION(node->type());
        POSTCONDITION(node->type()->properties() & typesystem::TypeProp::complete);

        mCurrFunc = nullptr;
    }

    bool visit(meta::ExprStatement* node) override {
        dispatch(TypeEvaluator{}, node->expression(), mScope);
        return false;
    }

    bool visit(VarDecl* node) override {
        POSTCONDITION(node->type());
        POSTCONDITION(node->type()->properties() & typesystem::TypeProp::complete);

        node->setType(mScope.findType(node->typeName()));
        if (!node->type())
            throw SemanticError(node, "Variable '%s' has unknown type '%s'", node->name(), node->typeName());
        if (!node->inited()) {
            if (!(node->type()->properties() & typesystem::TypeProp::complete))
                throw SemanticError(node, "Can't deduce variable '%s' type.", node->name());
            return false;
        }

        utils::optional<Type> initExprType = dispatch(TypeEvaluator{}, node->initExpr(), mScope);
        if (!(node->type()->properties() & typesystem::TypeProp::complete))
            node->setType(initExprType);
        else if (node->type() != initExprType)
            throw SemanticError(
                node, "Attempt to init variable '%s' of type '%s' with value of type '%s'",
                node->name(), node->type()->name(), initExprType->name()
            );
        return false;
    }

    bool visit(If* node) override {
        utils::optional<Type> condType = dispatch(TypeEvaluator{}, node->condition(), mScope);
        if (!(condType->properties() & typesystem::TypeProp::boolean))
            throw SemanticError(node->condition(), "If statement can't work with condition of type '%s'", condType->name());
        if (node->thenBlock())
            node->thenBlock()->walk(this);
        if (node->elseBlock())
            node->elseBlock()->walk(this);
        return false;
    }

    bool visit(meta::Return* node) override {
        utils::optional<Type> ret = node->value() == nullptr ?
            mScope.findType(typesystem::BuiltinType::Void):
            dispatch(TypeEvaluator{}, node->value(), mScope)
        ;
        if (!(mCurrFunc->type()->properties() & typesystem::TypeProp::complete)) {
            if (!(ret->properties() & typesystem::TypeProp::complete))
                throw SemanticError(node, "Can't return value of incomplete type");
            mCurrFunc->setType(ret);
        }
        else if (mCurrFunc->type() != ret)
            throw SemanticError(
                node, "Attempt to return value of type '%s' from function returning '%s'",
                ret->name(), mCurrFunc->type()->name()
            );
        return false;
    }

private:
    Scope& mScope;
    Function* mCurrFunc = nullptr;
};

utils::optional<Type> TypeEvaluator::operator() (Call* node, Scope& scope) {
    PRECONDITION(node->args().size() == node->function()->args().size());
    if (!node->function()->type()) {
        TypeChecker subchecker(scope);
        node->function()->walk(&subchecker);
    }
    const auto& argdecls = node->function()->args();
    const auto& args = node->args();
    for (size_t i = 0; i < args.size(); ++i) {
        utils::optional<Type> argtype = dispatch(*this, args[i], scope);
        if (argdecls[i]->type() != argtype)
            throw SemanticError(
                args[i], "Can't call function '%s' with argument %u of type '%s'. Expected type is '%s'",
                node->function()->name(), i, argtype->name(), argdecls[i]->type()->name()
            );
    }
    node->setType(node->function()->type());
    return node->type();
}

void checkTypes(AST* ast, Scope& scope) {
    TypeChecker visitor(scope);
    ast->walk(&visitor);
}

} // anonymous namespace
} // namespace meta::analysers
