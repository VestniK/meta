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

#include <experimental/string_view>

#include "utils/contract.h"

#include "parser/dictionary.h"
#include "parser/metaparser.h"
#include "parser/visibility.h"

namespace meta {

struct Package {
    std::experimental::string_view name;
    const Package* parent = nullptr;
};

template<typename T, typename Comparator = std::less<T>>
struct TargetComparator {
    bool operator() (const T* lhs, const T* rhs) const {return Comparator()(*lhs, *rhs);}
};

class Module {
public:
    Module() = default;
    ~Module() = default;
    Module(Module&&) = default;
    Module& operator= (Module&&) = default;
    Module(const Module&) = delete;
    const Module& operator= (const Module&) = delete;

    const Package* upsert(const Package* parent, std::experimental::string_view name) {
        Package pkg = {name, parent};
        const auto it = index.find(&pkg);
        if (it != index.end())
            return *it;
        packages.push_back(pkg);
        const Package* res = &packages.back();
        index.insert(res);
        return res;
    }

    auto begin() const {return index.cbegin();}
    auto end() const {return index.cend();}

private:
    struct IndexComparator {
        bool operator() (const Package& lhs, const Package& rhs) const {
            if (lhs.parent == rhs.parent)
                return lhs.name < rhs.name;
            if (!lhs.parent)
                return true;
            if (!rhs.parent)
                return false;
            return operator()(*lhs.parent, *rhs.parent);
        }
    };

    std::deque<Package> packages;
    std::set<const Package*, TargetComparator<Package, IndexComparator>> index;
};

class Actions : public ParseActions, public NodeActions
{
public:
    void package(const StackFrame *reduction, size_t size) override {
        assert(size == 3);
        mPackage = reduction[1].tokens;
        const Package* parent = nullptr;
        for (const Token tok: reduction[1].tokens) {
            if (tok.termNum != identifier)
                continue;
            parent = mModule.upsert(parent, tok);
        }
    }
    void changeVisibility(const StackFrame *reduction, size_t size) override;
    void onFunction(Function *node) override;
    void onSourceFile(SourceFile *node) override;

    Dictionary& dictionary() {return mDictionary;}
    const Module& module() const {return mModule;}

private:
    std::experimental::string_view mPackage;
    Visibility mDefaultVisibility = Visibility::Private;
    Dictionary mDictionary;
    Module mModule;
};

} // namespace meta
