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

#pragma once

#include <cinttypes>

namespace meta::utils {

template<typename E, typename T = uint64_t>
class Bitmask {
public:
    constexpr
    Bitmask() = default;
    constexpr
    Bitmask(E e): mVal(mask(e)) {}
    constexpr
    Bitmask(const Bitmask &other) = default;
    constexpr
    Bitmask & operator= (const Bitmask &other) = default;

    constexpr
    Bitmask operator| (Bitmask rhs) const {
        return Bitmask(mVal | rhs.mVal);
    }
    constexpr
    Bitmask operator| (E e) const {
        return Bitmask(mVal | mask(e));
    }
    constexpr
    Bitmask& operator|= (Bitmask rhs) {
        mVal |= rhs.mVal;
        return *this;
    }
    constexpr
    Bitmask& operator| (E e) {
        mVal |= mask(e);
        return *this;
    }

    constexpr
    Bitmask operator& (Bitmask rhs) const {
        return Bitmask(mVal & rhs.mVal);
    }
    constexpr
    Bitmask operator& (E e) const {
        return Bitmask(mVal & mask(e));
    }

    constexpr
    operator bool () const {
        return mVal != 0;
    }

private:
    constexpr
    Bitmask(T t): mVal(t) {}

    constexpr
    static T mask(E e) {
        return T(1) << static_cast<T>(e);
    }
private:
    T mVal = 0;
};

template<typename E, typename T = uint64_t>
constexpr
Bitmask<E, T> operator| (E lhs, Bitmask<E, T> rhs) {
    return Bitmask<E, T>(lhs) | rhs;
}

template<typename E, typename T = uint64_t>
constexpr
Bitmask<E, T> operator& (E lhs, Bitmask<E, T> rhs) {
    return Bitmask<E, T>(lhs) & rhs;
}

} // namespace meta::utils
