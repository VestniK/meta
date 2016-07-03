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

#include <cxxabi.h>
#include <malloc.h>
#include <execinfo.h>

#include "utils/exception.h"
#include "utils/types.h"

namespace meta {
namespace utils {

Exception::Exception(): std::exception() {
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
    for (int i = 0; i < traceSize; ++i) {
        string_view btLine = strings.get()[i];
        const size_t fnameStart = btLine.find('(');
        const size_t fnameEnd = btLine.rfind('+');
        if (fnameStart == string_view::npos || fnameEnd == string_view::npos || fnameStart + 1 == fnameEnd) {
            mBacktrace.push_back(std::string(btLine));
            continue;
        }
        std::string fname = {btLine.begin() + fnameStart + 1, btLine.begin() + fnameEnd};
        int status = 0;
        std::unique_ptr<char, decltype(&::free)> strbuf = {
            abi::__cxa_demangle(fname.c_str(), nullptr, nullptr, &status),
            &::free
        };
        if (status != 0) {
            mBacktrace.push_back(std::string(btLine));
            continue;
        }
        std::string str = {btLine.begin(), btLine.begin() + fnameStart + 1};
        str += strbuf.get();
        str.append(btLine.begin() + fnameEnd, btLine.end());

        mBacktrace.push_back(str);
    }
}

} // namespace utils
} // namespace meta
