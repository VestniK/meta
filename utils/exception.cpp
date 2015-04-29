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
#include <memory>

#include <malloc.h>
#include <execinfo.h>

#include "utils/exception.h"

namespace meta {
namespace utils {

Exception::Exception(): std::exception()
{
    constexpr size_t sizeinc = 128;
    constexpr size_t sizelimit = 1024;
    std::vector<void*> stacktrace(sizeinc);
    int traceSize = 0;
    while (
        (traceSize = ::backtrace(stacktrace.data(), stacktrace.size())) >= static_cast<int>(stacktrace.size()) &&
        stacktrace.size() <= sizelimit
    )
        stacktrace.resize(stacktrace.size() + sizeinc);
    mBacktrace.reserve(stacktrace.size());
    std::unique_ptr<char*, decltype(&::free)> strings = {::backtrace_symbols(stacktrace.data(), traceSize), &::free};
    if (!strings)
        return;
    for (int i = 0; i < traceSize; ++i)
        mBacktrace.push_back(strings.get()[i]);
}

} // namespace utils
} // namespace meta
