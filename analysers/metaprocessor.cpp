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

#include "utils/contract.h"

#include "parser/annotation.h"
#include "parser/metaparser.h"
#include "parser/function.h"

#include "analysers/metaprocessor.h"
#include "analysers/semanticerror.h"

namespace analysers {

void processMeta(meta::AST *ast)
{
    meta::walkTopDown<meta::Annotation>(*ast, [] (meta::Annotation *node) {
        PRECONDITION(node->target() != nullptr);

        /// @todo process user defined metas here
        auto attr = meta::Function::attribute(node->name());
        if (attr == meta::Function::invalid)
            throw SemanticError(node, "Invalid attribute '%s'", node->name().c_str());
        node->target()->set(attr);
        return false;
    });
}

} // namespace analysers
