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

#include <algorithm>

#include "utils/types.h"

#include "parser/function.h"

#include "generators/llvmgen/mangling.h"

namespace meta {
namespace generators {
namespace llvmgen {

std::string mangledName(Function* func) {
    if (func->is(Function::entrypoint))
        return "main";
    if (func->mangledName() != utils::nullopt)
        return static_cast<std::string>(*func->mangledName());
    // Perform mangling based on regular rules
    std::string res;
    res.reserve(func->package().size() + func->name().size() + 1);
    std::transform(
        func->package().begin(), func->package().end(), std::back_inserter(res),
        [](char ch) {return ch == '.' ? '_' : ch;}
    );
    res.push_back('_');
    std::copy(func->name().begin(), func->name().end(), std::back_inserter(res));
    return res;
}

} // namespace abi
} // namespace generators
} // namespace meta
