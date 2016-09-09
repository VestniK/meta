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

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <vector>

namespace meta {
namespace utils {

template<typename T, size_t N>
using array_t = T[N];

template<typename T, size_t N>
constexpr
size_t array_size(const utils::array_t<T, N>&) {return N;}

template<typename T>
class array_view {
public:
    constexpr array_view() = default;
    constexpr array_view(const T* data, size_t size): mData(data), mSize(size) {}
    array_view(const std::vector<T>& data): mData(data.data()), mSize(data.size()) {}
    template<size_t N>
    constexpr array_view(const array_t<const T, N>& arr): mData(arr), mSize(N) {}

    constexpr const T* data() const {return mData;}
    constexpr size_t size() const {return mSize;}

    const T& operator[] (size_t pos) const {
        assert(pos < mSize);
        return mData[pos];
    }

    constexpr const T* begin() const {return mData;}
    constexpr const T* end() const {return mData + mSize;}

    static constexpr size_t npos = std::numeric_limits<size_t>::max();

    array_view slice(size_t from, size_t to = npos) {
        array_view res;
        if (from >= mSize || to <= from)
            return res;
        res.mData = mData + from;
        res.mSize = std::min(to, mSize) - from;
        return res;
    }

private:
    const T* mData = nullptr;
    size_t mSize = 0;
};

} // namespace utils
} // namespace meta
