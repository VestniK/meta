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

#include <deque>
#include <set>

#include "utils/contract.h"
#include "utils/types.h"

#include "parser/dictionary.h"
#include "parser/metaparser.h"
#include "parser/visibility.h"

namespace meta {

struct Package {
    utils::string_view name;

    utils::optional<Package> parent() const {
        const char* endPos = name.data() + name.size() - 1;
        while (endPos != name.data() && *endPos != '.')
            --endPos;
        if (endPos == name.data())
            return utils::nullopt;

        return Package{{name.data(), static_cast<size_t>(endPos - name.data()) - 1}};
    }

    bool operator< (const Package& rhs) const {return name < rhs.name;}
};

class Module {
public:
    Module() = default;
    ~Module() = default;
    Module(Module&&) = default;
    Module& operator= (Module&&) = default;
    Module(const Module&) = delete;
    const Module& operator= (const Module&) = delete;

    const Package& package(const utils::string_view& name) {
        Package pkg = {name};
        auto it = mPackages.find(pkg);
        if (it != mPackages.end())
            return *it;
        auto res = mPackages.insert(pkg);
        return *res.first;
    }

    const auto& packages() const {return mPackages;}

private:
    std::set<Package> mPackages;
};

class Actions : public ParseActions, public NodeActions
{
public:
    void package(const StackFrame *reduction, size_t size) override {
        PRECONDITION(size == 3);
        POSTCONDITION(mCurrentPackage != nullptr);
        mCurrentPackage = &mModule.package(reduction[1].tokens);
    }
    void changeVisibility(const StackFrame *reduction, size_t size) override;
    void onFunction(Function *node) override;
    void onSourceFile(SourceFile *node) override;

    Dictionary& dictionary() {return mDictionary;}
    const Module& module() const {return mModule;}

private:
    const Package* mCurrentPackage = nullptr;
    Visibility mDefaultVisibility = Visibility::Private;
    Dictionary mDictionary;
    Module mModule;
};

} // namespace meta
