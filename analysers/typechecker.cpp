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

#include "parser/function.h"
#include "parser/metaparser.h"
#include "parser/number.h"
#include "parser/vardecl.h"

#include "typesystem/type.h"
#include "typesystem/typesstore.h"

#include "analysers/typechecker.h"

namespace {

/// @todo use Translator instead of visitor, assign types to operations as well and check type consistency
class TypeCheckVisitor: public meta::Visitor
{
public:
    explicit TypeCheckVisitor(typesystem::TypesStore &types): mTypes(types) {}

    virtual void leave(meta::VarDecl *node) override
    {
        node->setType(mTypes.getByName(node->typeName()));
    }

    virtual void leave(meta::Number *node) override
    {
        node->setType(mTypes.getPrimitive(typesystem::Type::Int));
    }

    virtual bool visit(meta::Function *node)
    {
        node->setType(mTypes.getByName(node->retType()));
    }

private:
    typesystem::TypesStore &mTypes;
};

} // anonymous namespace

namespace analysers {

void checkTypes(meta::AST *ast, typesystem::TypesStore &types)
{
    TypeCheckVisitor visitor(types);
    ast->walk(&visitor);
}

} // namespace analysers
