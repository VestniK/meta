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

#include "utils/exception.h"

#include "parser/metalexer.h"
#include "parser/metaparser.h"

namespace meta {
class Node;

class NodeException: public utils::Exception {
public:
    ~NodeException() = default;

    const char *what() const noexcept override {return mMsg.c_str();}
    const TokenSequence &tokens() const {return mTokens;}
    const std::string &sourcePath() const {return mSrc;}

protected:
    NodeException(Node *node, const std::string &msg, std::vector<std::string>&& backtrace):
        utils::Exception(std::move(backtrace)), mMsg(msg),
        mTokens(node->tokens()), mSrc(node->sourceLocation())
    {
        mTokens.detach(mErrContext);
    }

private:
    std::string mMsg;
    std::string mErrContext;
    TokenSequence mTokens;
    std::string mSrc;
};

} // namespace meta
