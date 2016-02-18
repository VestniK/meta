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

#include <cassert>
#include <fstream>
#include <stdexcept>

#include "fs/io.h"

std::vector<char> readWholeFile(const meta::utils::fs::path& path) {
    std::vector<char> res;
    std::ifstream in(path, std::ifstream::in | std::ifstream::binary);
    if (!in)
        throw std::system_error(errno, std::system_category(), "Failed to open file: " + path.string());
    const auto fileSize = meta::utils::fs::file_size(path);
    res.resize(static_cast<std::size_t>(fileSize + 1));
    in.read(res.data(), fileSize);
    if (!in.good())
        throw std::system_error(errno, std::system_category(), "Failed to read file content: " + path.string());
    assert(in.gcount() == static_cast<std::streamsize>(fileSize));
    res[fileSize] = 0;
    return res;
}
