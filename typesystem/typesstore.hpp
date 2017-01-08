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
#include "typesystem/type.h"
#include "typesystem/typesstore.h"

namespace meta::typesystem {

TypesStore::TypesStore(TypesStore* parent): mParent(parent) {
    for (auto& primitive: createBuiltinTypes()) {
        const auto name = primitive->name();
        mTypes[name] = std::move(primitive);
    }
}

Type* TypesStore::get(utils::string_view name) const {
    const auto it = mTypes.find(name);
    if (it == mTypes.end())
        return mParent ? mParent->get(name) : nullptr;
    return it->second.get();
}

} // namespace meta::typesystem
