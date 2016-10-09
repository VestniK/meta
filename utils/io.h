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

#include <fstream>
#include <iterator>
#include <string>
#include <system_error>

#include "utils/types.h"

namespace meta {
namespace utils {

enum class IO {in, out};

namespace detail {
template<IO dir>
struct FStreamChooser;

template<>
struct FStreamChooser<IO::in> {
    using type = std::ifstream;
};

template<>
struct FStreamChooser<IO::out> {
    using type = std::ofstream;
};
}

template<IO dir>
using FStream = typename detail::FStreamChooser<dir>::type;

template<IO dir>
FStream<dir> open(const fs::path& path, std::ios_base::openmode mode) {
    FStream<dir> res(path, mode);
    if (!res)
        throw std::system_error(errno, std::system_category(), path.string());
    return res;
}

inline
std::string readAll(const fs::path& path) {
    auto in = open<IO::in>(path, std::ifstream::in | std::ifstream::binary);
    in.unsetf(std::ifstream::skipws);
    return {std::istream_iterator<char>(in), std::istream_iterator<char>()};
}

}} // namespace meta::utils
