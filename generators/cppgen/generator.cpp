/*
 * Meta language compiler
 * Copyright (C) 2016  Sergey Vidyuk <sir.vestnik@gmail.com>
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

#include <iostream>
#include <fstream>

#include "utils/contract.h"

#include "typesystem/type.h"

#include "parser/metaparser.h"
#include "parser/function.h"
#include "parser/vardecl.h"

#include "cppwriter.h"
#include "generator.h"

namespace meta {
namespace generators{
namespace cppgen {

void generate(
    const Dictionary& dict,
    const utils::fs::path& outputHeader,
    const utils::fs::path& //outputCpp
) {
    std::ofstream out(outputHeader, std::fstream::trunc | std::fstream::binary);
    if (!out)
        throw std::system_error(errno, std::system_category(), "Failed to open " + outputHeader.string());
    CppWriter writer(out);

    for (const auto& kv: dict) {
        const auto& functions = kv.second;

        writer.setPackage(kv.first);
        for (const auto& func: functions) {
            Function* f = func.second;
            if (f->visibility() != Visibility::Export)
                continue;
            writer.forwardDeclare(f);
        }
    }
}

}}} // namespace meta::generators::cppgen
