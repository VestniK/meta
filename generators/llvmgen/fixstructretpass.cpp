/*
 * Meta language compiler
 * Copyright (C) 2016 Sergey Vidyuk <sir.vestnik@gmail.com>
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

#include <llvm/Analysis/CallGraphSCCPass.h>

#include "generators/llvmgen/fixstructretpass.h"

namespace meta::generators::llvmgen {

namespace {

class FixStructRet: public llvm::CallGraphSCCPass {
public:
    static char ID;
    FixStructRet(): llvm::CallGraphSCCPass(ID) {}

    bool runOnSCC(llvm::CallGraphSCC& SCC) override {
        return false;
    }
};

} // anonymous namespace

char FixStructRet::ID = 0;
llvm::RegisterPass<FixStructRet> reg(
    "fixstructret",
    "Fix passing and returning struct to and from functions to satisfy targer platform C ABI"
);

llvm::Pass* createFixStructRetPass() {
    return new FixStructRet;
}

} // namespace meta::generators::llvmgen
