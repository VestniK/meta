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

#include <iterator>
#include <string>

#include <gtest/gtest.h>

#include "utils/types.h"

#include "parser/actions.h"

using namespace meta;

std::ostream& operator<< (std::ostream& out, const meta::Package* const& pkg)
{
    if (!pkg)
        return out << "null";
    if (!pkg->parent)
        return out << pkg->name;
    return out << pkg->parent << '.' << pkg->name;
}

class ModuleTests: public ::testing::Test
{
public:
    void SetUp() override
    {
        for (auto package: packages) {
            const meta::Package* pkg = nullptr;
            while (!package.empty()) {
                auto dot = package.find('.');
                const utils::string_view part = package.substr(0, dot);
                pkg = module.upsert(pkg, part);
                package = dot != utils::string_view::npos ? package.substr(dot + 1) : "";
            }
        }
    }

    meta::Module module;
private:
    const std::vector<utils::string_view> packages = {
        "A.A1.A11",
        "A.A1.A12",
        "A.A2.A21",
        "A.A2.A22",
        "A.A2.A23",
        "A.A3",
        "B.B1",
        "B.B2"
    };
};

TEST_F(ModuleTests, treeTraverse)
{
//     const meta::Package* prevPkg = nullptr;
    for(const meta::Package* pkg :module) {
        std::cout << pkg->name << std::endl;
//         ASSERT_TRUE(pkg->parent == nullptr || pkg->parent == prevPkg) << "pkg: '" << pkg->name << "'; prevPkg: '" << prevPkg->name << "'";
//         prevPkg = pkg;
    }
}
