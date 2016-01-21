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

std::vector<char> readWholeFile(const std::string &path)
{
    std::vector<char> res;
    std::ifstream in(path, std::ifstream::in | std::ifstream::binary);
    if (!in)
        throw std::runtime_error("Failed to open file: " + path);
    in.seekg(0, std::ifstream::end);
    std::streampos fileSize = in.tellg();
    res.resize(std::size_t(fileSize) + 1);
    in.seekg(0, std::ifstream::beg);
    in.read(res.data(), fileSize);
    if (!in.good())
        throw std::runtime_error("Failed to read file content: " + path);
    assert(in.gcount() == fileSize);
    res[fileSize] = 0;
    return res;
}
