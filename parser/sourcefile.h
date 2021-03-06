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
#pragma once

#include "parser/metaparser.h"

namespace meta {

class SourceFile: public Visitable<Node, SourceFile> {
public:
    SourceFile(const utils::SourceFile& src, utils::array_view<StackFrame> reduction);

    void setPackage(const utils::string_view& val) {mPackage = val;}
    const utils::string_view& package() const {return mPackage;}

    void walk(Visitor* visitor, int depth) override {
        if (this->accept(visitor) && depth != 0) {
            for (auto child: mChildren)
                child->walk(visitor, depth - 1);
        }
        this->seeOff(visitor);
    }

private:
    std::vector<Node::Ptr<Node>> mChildren;
    utils::string_view mPackage;
};

} // namespace meta
