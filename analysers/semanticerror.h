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

#ifndef SEMANTICERROR_H
#define SEMANTICERROR_H

#include <exception>
#include <string>

#include "parser/metalexer.h"

namespace meta {

class Node;

} // namespace meta

namespace analysers {

class SemanticError: public std::exception
{
public:
    SemanticError(meta::Node *node, const char *format, ...) __attribute__((format(printf, 3, 4)));
    ~SemanticError();

    virtual const char *what() const noexcept override;
    const meta::TokenSequence &tokens() const {return mTokens;}

private:
    std::string mMsg;
    std::string mErrContext;
    meta::TokenSequence mTokens;
};

} // namespace analysers

#endif // SEMANTICERROR_H
