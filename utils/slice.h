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

#include <map>

namespace meta::utils {

template<typename Iter>
struct Slice {
    Iter begin_it;
    Iter end_it;

    Iter begin() const {return begin_it;}
    Iter end() const {return end_it;}
    bool empty() const {return !(begin_it != end_it);}
};

template<typename Iter>
auto slice(Iter b, Iter e) {return Slice<Iter>{b, e};}

template<typename Iter>
auto slice(Iter b) {Slice<Iter> res{b, b}; ++res.end_it; return res;}

template<typename Iter>
auto slice(std::pair<Iter, Iter> range) {return Slice<Iter>{range.first, range.second};}

} // namespace meta::utils
