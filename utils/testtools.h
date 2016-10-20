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

#include "utils/sourcefile.h"
#include "utils/types.h"

#if !defined(PARSER_ONLY_TEST)
#   include "analysers/semanticerror.h"
#endif

#if defined(PARSER_ONLY_TEST)
#   define CATCH_SEMANTIC_ERROR
#else
#   define CATCH_SEMANTIC_ERROR \
        catch (const ::meta::analysers::SemanticError& err) { \
            FAIL() << \
                err.sourcePath() << ':' << err.tokens().begin()->line << \
                ':' << err.tokens().begin()->column << ": " << err.what() << ":\n" << \
                err.tokens().lineStr() << "...\n" << \
                meta::utils::markerLine(static_cast<size_t>(err.tokens().colnum())); \
        }

#   define ASSERT_ANALYSE(statement) \
        try { \
            statement; \
        } CATCH_SEMANTIC_ERROR

#endif

#define ASSERT_PARSE(parser, source) \
    try { \
        parser.parse(source); \
    } catch (const ::meta::SyntaxError& err) { \
        FAIL() << \
            err.sourcePath() << ':' << err.token().line << ':' << \
            err.token().column << ": " << err.what() << ":\n" << \
            err.line() << "\nExpected one of the following terms:\n" << \
            err.expected() << "Parser stack dump:\n" << err.parserStack(); \
    } CATCH_SEMANTIC_ERROR

namespace meta::utils {

inline
std::string markerLine(size_t column) {
    std::string res(column, ' ');
    res.back() = '^';
    return res;
}

struct ErrorTestData {
    SourceFile input;
    string_view errMsg;
};

inline
std::ostream& operator<< (std::ostream& out, const ErrorTestData& dat) {
    out <<
        "=== input ===\n" <<
        dat.input.content() <<
        "\n=== expected error ===\n" <<
        dat.errMsg <<
        "\n======";
    return out;
}

class ErrorTest: public ::testing::TestWithParam<ErrorTestData> {};

} // namespace meta::utils

namespace std::experimental {
inline
namespace fundamentals_v1 {
inline
void PrintTo(const string_view& str, std::ostream* dest) {(*dest) << str;}
}}
