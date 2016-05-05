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

#include <cassert>
#include <cstddef>

namespace meta {
namespace utils {

template<typename T>
class array_view {
public:
    array_view() = default;
    array_view(const T* data, size_t size): mData(data), mSize(size) {}

    const T* data() const {return mData;}
    size_t size() const {return mSize;}

    const T& operator[] (size_t pos) const {
        assert(pos < mSize);
        return mData[pos];
    }

    const T* begin() const {return mData;}
    const T* end() const {return mData + mSize;}

private:
    const T* mData = nullptr;
    size_t mSize = 0;
};

} // namespace utils
} // namespace meta
