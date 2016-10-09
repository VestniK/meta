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
#include "utils/bitmask.h"

#include "parser/annotation.h"
#include "parser/codeblock.h"
#include "parser/declaration.h"
#include "parser/metaparser.h"
#include "parser/typed.h"
#include "parser/vardecl.h"
#include "parser/visibility.h"

namespace meta {

enum class FuncFlags {
    entrypoint
};

class Function: public Visitable<Declaration, Function>, public Typed {
public:
    Function(const utils::SourceFile& src, utils::array_view<StackFrame> reduction);
    ~Function();

    const Declaration::AttributesMap &attributes() const override {return attrMap;}

    const utils::string_view &retType() const {return mRetType;}
    const utils::string_view &package() const {return mPackage;}
    void setPackage(const utils::string_view &pkg) {mPackage = pkg;}
    void setMangledName(const utils::string_view &val) {mMangledName = val;}
    void setMangledName(std::nullptr_t) {mMangledName = utils::nullopt;}
    const utils::optional<utils::string_view>& mangledName() const {return mMangledName;}
    const auto& args() const {return mArgs;}
    CodeBlock* body() {return mBody;}

    Visibility visibility() const {return mVisibility;}
    void setVisibility(Visibility val) {mVisibility = val;}

    const auto& flags() const {return mFlags;}
    auto& flags() {return mFlags;}

    void walk(Visitor* visitor, int depth = infinitDepth) override {
        if (accept(visitor) && depth != 0) {
            for (auto& ann: mAnnotations)
                ann->walk(visitor, depth - 1);
            for (auto& arg: mArgs)
                arg->walk(visitor, depth - 1);
            if (mBody)
                mBody->walk(visitor, depth - 1);
        }
        seeOff(visitor);
    }

private:
    std::vector<Node::Ptr<Annotation>> mAnnotations;
    std::vector<Node::Ptr<VarDecl>> mArgs;
    Node::Ptr<CodeBlock> mBody;
    utils::string_view mPackage;
    utils::string_view mRetType;
    utils::optional<utils::string_view> mMangledName;
    Visibility mVisibility = Visibility::Default;
    utils::Bitmask<FuncFlags> mFlags;

    static const Declaration::AttributesMap attrMap;
};

} // namespace meta
