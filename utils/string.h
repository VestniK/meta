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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "utils/types.h"

namespace meta {
namespace utils {

/**
 * @return nullopt if string is not a number
 * @todo rework using std::error_code approach and provide trivial throwing overload
 * @todo handle automatic base detection for 0x prefix and b suffix
 */
template<typename IntType>
optional<IntType> number(const string_view& str, uint8_t base = 10) {
    IntType res = 0;
    for (char ch: str) {
        res = res*base;
        if (ch >= '0' && ch <= '9' && ch - '0' < base)
            res += ch - '0';
        else if (ch >= 'a' && ch - 'a' + 10 < base)
            res += ch - 'a' + 10;
        else if (ch >= 'A' && ch - 'A' + 10 < base)
            res += ch - 'A' + 10;
        else
            return nullopt;
    }
    return res;
}

} // namespace utils
} // namespace meta
