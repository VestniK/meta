/*
 * Meta language compiler
 * Copyright (C) 2015  Sergey Vidyuk <sir.vestnik@gmail.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#include <cstdarg>
#include <cstdio>

#include "parser/metaparser.h"

#include "analysers/unexpectednode.h"

namespace meta {
namespace analysers {

UnexpectedNode::UnexpectedNode(Node *node, const char *format, ...):
    NodeException(node)
{
    va_list args;
    va_start(args, format);
    const int size = vsnprintf(nullptr, 0, format, args) + 1;
    char buf[size];
    va_start(args, format);
    vsnprintf(buf, size, format, args);
    setMsg(std::string(buf, size));
}

UnexpectedNode::~UnexpectedNode()
{
}

} // namespace analysers
} // namespace meta
