/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Сергей Видюк <sir.vestnik@gmail.com>
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
#include <string>

#include "parser/actions.h"
#include "parser/function.h"

void Actions::package(const meta::StackFrame *reduction, size_t size)
{
    assert(size == 4);
    std::string pkgName = reduction[1].tokens;
    for (auto node : reduction[3].nodes)
        node->walkTopDown<meta::Function>([&](meta::Function *func){func->setPackage(pkgName); return true;}, 0);
}
