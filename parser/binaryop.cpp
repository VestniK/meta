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

#include <cassert>

#include "parser/binaryop.h"

namespace {

const char *operationsMap[meta::BinaryOp::operationsCount] = {
    // Arythmetics
    "+", "-", "*", "/",
    // Comparisions
    "==", "!=", "<", ">", "<=", ">="
};

}

namespace meta {

BinaryOp::BinaryOp(AST *ast, const StackFrame *reduction, size_t size): Node(ast, reduction, size)
{
    assert(size == 3);
    std::string opStr = reduction[1].tokens;
    int opCode = 0;
    for (; opCode < operationsCount; ++opCode) {
        if (opStr == operationsMap[opCode])
            break;
    }
    assert(opCode < operationsCount);
    mOp = static_cast<Operation>(opCode);
}

} // namespace meta
