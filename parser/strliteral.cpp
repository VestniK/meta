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

StrLiteral::StrLiteral(utils::array_view<StackFrame> reduction):
    Visitable<Expression, StrLiteral>(reduction)
{
    PRECONDITION(reduction.size() == 1);
    PRECONDITION(reduction[0].symbol == strLiteral);
    auto token = *reduction[0].tokens.begin();
    bool escape = false;
    for (const char *cur = token.start + 1; cur < token.end - 1; ++cur) {
        if (!escape && *cur == '\\') {
            escape = true;
            continue;
        }
        if (escape) {
            switch (*cur) {
                case '\\': mVal.push_back('\\'); break;
                case '"': mVal.push_back('"'); break;
                case 'n': mVal.push_back('\n'); break;
                case 'r': mVal.push_back('\r'); break;
                case 't': mVal.push_back('\t'); break;
                case 'a': mVal.push_back('\a'); break;
                case 'b': mVal.push_back('\b'); break;
                case 'f': mVal.push_back('\f'); break;
                case '0': mVal.push_back('\0'); break;
                default: mVal.push_back('\\'); mVal.push_back(*cur);
            }
            escape = false;
            continue;
        }
        mVal.push_back(*cur);
    }
}

} // namespace meta
