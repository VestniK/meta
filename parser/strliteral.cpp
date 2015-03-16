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

#include "parser/strliteral.h"

namespace meta {

StrLiteral::StrLiteral(const StackFrame *reduction, size_t size): Node(reduction, size)
{
    PRECONDITION(size == 1);
    PRECONDITION(reduction[0].symbol == meta::strLiteral);
    auto token = *reduction[0].tokens.begin();
    bool escape = false;
    for (const char *cur = token.start + 1; cur < token.end - 1; ++cur) {
        if (!escape && *cur == '\\') {
            escape = true;
            continue;
        }
        if (escape) {
            switch (*cur) {
                case '\\': mVal += '\\'; break;
                case '"': mVal += '"'; break;
                case 'n': mVal += '\n'; break;
                case 'r': mVal += '\r'; break;
                case 't': mVal += '\t'; break;
                case 'a': mVal += '\a'; break;
                case 'b': mVal += '\b'; break;
                case 'f': mVal += '\f'; break;
                case '0': mVal += '\0'; break;
                default: mVal += '\\'; mVal += *cur;
            }
            continue;
        }
        mVal += *cur;
    }
}

} // namespace meta
