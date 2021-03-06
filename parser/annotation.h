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

#include "utils/types.h"

#include "parser/metaparser.h"

namespace meta {

class Declaration;

class Annotation: public Visitable<Node, Annotation> {
public:
    Annotation(const utils::SourceFile& src, utils::array_view<StackFrame> reduction):
        Visitable<Node, Annotation>(src, reduction)
    {
        PRECONDITION(reduction.size() == 1);
        PRECONDITION(reduction[0].tokens.begin() != reduction[0].tokens.end());
        PRECONDITION(reduction[0].tokens.begin()->termNum == annotation);
        PRECONDITION(countNodes(reduction) == 0);

        Token token = *(reduction[0].tokens.begin());
        ++token.start; // skip '@' character in the beggining of annotation
        mName = token;
    }

    const utils::string_view& name() const {return mName;}

    void setTarget(Declaration* val) {mTarget = val;}
    Declaration* target() {return mTarget;}

    void walk(Visitor* visitor, int) override {
        accept(visitor);
        // Annotation have no children yet
        seeOff(visitor);
    }

private:
    utils::string_view mName;
    Declaration* mTarget;
};

} // namespace meta
