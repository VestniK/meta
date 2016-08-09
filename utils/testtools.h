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

#include <ostream>

#include <gtest/gtest.h>

#include "utils/types.h"

namespace meta::utils {

/// @todo add SemanticError dump and think how to remove dependency on parser and analysers
#define ASSERT_PARSE(parser, srcPath, input) \
    try { \
        parser.parse(srcPath, input); \
    } catch (const SyntaxError& err) { \
        FAIL() << \
            err.sourcePath() << ':' << err.token().line << ':' << \
            err.token().column << ": " << err.what() << ":\n" << \
            err.line() << "\nExpected one of the following terms:\n" << \
            err.expected() << "Parser stack dump:\n" << err.parserStack(); \
    }

struct ErrorTestData {
    utils::string_view input;
    utils::string_view errMsg;
};

inline
std::ostream& operator<< (std::ostream& out, const ErrorTestData& dat) {
    out << "=== input ===\n" << dat.input << "\n=== expected error ===\n" << dat.errMsg << "\n======";
    return out;
}

class ErrorTest: public ::testing::TestWithParam<ErrorTestData> {};

} // namespace meta::utils
