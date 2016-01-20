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
#pragma once

#include <string>

#include "parser/metalexer.h"

#include "analysers/nodeexception.h"

namespace meta {
namespace analysers {

class SemanticError: public NodeException
{
public:
    [[gnu::format(printf, 3, 4)]]
    SemanticError(Node *node, const char *format, ...);
    ~SemanticError();
};

} // namespace analysers
} // namespace meta

