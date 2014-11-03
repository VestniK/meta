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

#ifndef ACTIONS_H
#define ACTIONS_H

#include <string>

#include "parser/dictionary.h"
#include "parser/metaparser.h"
#include "parser/visibility.h"

namespace meta {

class Actions : public meta::ParseActions, public meta::NodeActions
{
public:
    virtual void package(const meta::StackFrame *reduction, size_t size) override;
    virtual void changeVisibility(const meta::StackFrame *reduction, size_t size) override;
    virtual void onFunction(meta::Function *node) override;
    virtual void onSourceFile(meta::SourceFile *node) override;

    Dictionary &dictionary() {return mDictionary;}

private:
    std::string mPackage;
    meta::Visibility mDefaultVisibility = meta::Visibility::Private;
    Dictionary mDictionary;
};

} // namespace meta

#endif // ACTIONS_H
