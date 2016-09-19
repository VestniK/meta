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
 */
#pragma once

#include <exception>
#include <string>
#include <vector>

#include "utils/array_view.h"

namespace meta {
namespace utils {

std::vector<std::string> captureBacktrace();

class Exception: public std::exception {
public:
    Exception(std::vector<std::string>&& backtrace): mBacktrace(std::move(backtrace)) {}
    virtual ~Exception() = default;

    utils::array_view<std::string> backtrace() const {return mBacktrace;}

private:
    std::vector<std::string> mBacktrace;
};

} // namespace utils
} // namespace meta
