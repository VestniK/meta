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

#ifndef FUNCTION_H
#define FUNCTION_H

#include <string>

#include <boost/optional.hpp>

#include "parser/declaration.h"
#include "parser/metaparser.h"
#include "parser/typed.h"
#include "parser/visibility.h"

namespace meta {

class Function: public Declaration, public Typed
{
meta_NODE
public:
    Function(const StackFrame *start, size_t size);

    enum Attribute {
        invalid = 0,
        entrypoint = (1<<0)
    };

    virtual const Declaration::AttributesMap &attributes() const override {return attrMap;}
    virtual Function *asFunction() override {return this;}

    const std::string &name() const {return mName;}
    const std::string &retType() const {return mRetType;}
    const std::string &package() const {return mPackage;}
    void setPackage(const std::string &pkg) {mPackage = pkg;}
    void setMangledName(const std::string &val) {mMangledName = val;}
    void setMangledName(std::nullptr_t) {mMangledName = boost::none;}
    const std::string *mangledName() const {return mMangledName.get_ptr();}
    std::vector<VarDecl*> args();
    CodeBlock *body();

    Visibility visibility() const {return mVisibility;}
    void setVisibility(Visibility val) {mVisibility = val;}

    void set(Attribute attr, bool val = true);
    bool is(Attribute attr) const;

private:
    std::string mPackage;
    std::string mName;
    std::string mRetType;
    boost::optional<std::string> mMangledName;
    Visibility mVisibility = Visibility::Default;
    int mAttributes = 0;

    static const Declaration::AttributesMap attrMap;
};

} // namespace meta

#endif // FUNCTION_H

