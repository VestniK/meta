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

#include <experimental/string_view>
#include <experimental/optional>

#include "parser/declaration.h"
#include "parser/metaparser.h"
#include "parser/typed.h"
#include "parser/visibility.h"

namespace meta {

class Function: public Visitable<Declaration, Function>, public Typed
{
public:
    Function(const StackFrame *start, size_t size);

    enum Attribute {
        invalid = 0,
        entrypoint = (1<<0)
    };

    virtual const Declaration::AttributesMap &attributes() const override {return attrMap;}

    const std::experimental::string_view &name() const {return mName;}
    const std::experimental::string_view &retType() const {return mRetType;}
    const std::experimental::string_view &package() const {return mPackage;}
    void setPackage(const std::experimental::string_view &pkg) {mPackage = pkg;}
    void setMangledName(const std::experimental::string_view &val) {mMangledName = val;}
    void setMangledName(std::nullptr_t) {mMangledName = std::experimental::nullopt;}
    const std::experimental::optional<std::experimental::string_view>& mangledName() const {return mMangledName;}
    std::vector<VarDecl*> args();
    CodeBlock *body();

    Visibility visibility() const {return mVisibility;}
    void setVisibility(Visibility val) {mVisibility = val;}

    void set(Attribute attr, bool val = true);
    bool is(Attribute attr) const;

private:
    std::experimental::string_view mPackage;
    std::experimental::string_view mName;
    std::experimental::string_view mRetType;
    std::experimental::optional<std::experimental::string_view> mMangledName;
    Visibility mVisibility = Visibility::Default;
    int mAttributes = 0;

    static const Declaration::AttributesMap attrMap;
};

} // namespace meta
