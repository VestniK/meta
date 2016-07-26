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

#include "parser/codeblock.h"

namespace meta {

CodeBlock::CodeBlock(utils::array_view<StackFrame> reduction):
    Visitable<Node, CodeBlock>(reduction),
    mChildren(getNodes(reduction))
{
}

void CodeBlock::add(Node* statement) {
    /// @todo allow to add statements only, not arbitrary node
    mChildren.emplace_back(statement);
}

const std::vector<Node::Ptr<Node>>& CodeBlock::statements() const {
    return mChildren;
}

} // namespace meta
