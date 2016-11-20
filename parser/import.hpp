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
#include "utils/contract.h"

#include "parser/import.h"

namespace meta {

Import::Import(const utils::SourceFile& src, utils::array_view<StackFrame> reduction):
    Visitable<Declaration, Import>(src, reduction)
{
    // {'import', <qname>, ';'} OR {'import', <qname>, 'as' <identifier>, ';'}
    PRECONDITION(reduction.size() == 3 || reduction.size() == 5);
    PRECONDITION(countNodes(reduction) == 0);

    utils::string_view target = reduction[1].tokens;
    const size_t splitpos = target.rfind('.');
    mPackage = target.substr(0, splitpos);
    mTarget = splitpos != utils::string_view::npos ?
        target.substr(splitpos + 1):
        utils::string_view{}
    ;
    if (reduction.size() == 5)
        mName = reduction[3].tokens;
    else
        mName = mTarget;
}

const Declaration::AttributesMap& Import::attributes() const {
    static AttributesMap attrmap;
    return attrmap;
}

} // namespace meta

