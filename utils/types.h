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

#include <experimental/filesystem>
#include <experimental/optional>
#include <experimental/string_view>

#include "utils/array_view.h"

using namespace std::experimental::literals::string_view_literals;

namespace meta {
namespace utils {

using std::experimental::string_view;

using std::experimental::optional;
using std::experimental::nullopt;

namespace fs = std::experimental::filesystem;

} // namespace utils
} // namespace meta
