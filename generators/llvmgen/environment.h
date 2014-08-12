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

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <memory>

#include "generators/llvmgen/privateheadercheck.h"

namespace llvm {

class LLVMContext;
class Module;

}

namespace meta {

class Function;

}

namespace generators {
namespace llvmgen {

struct Environment
{
    Environment(const std::string &moduleName);
    ~Environment();

    llvm::Function *addFunction(meta::Function *func);

    llvm::LLVMContext &context;
    std::unique_ptr<llvm::Module> module;
};

} // namespace llvmgen
} // namespace generators

#endif // ENVIRONMENT_H

