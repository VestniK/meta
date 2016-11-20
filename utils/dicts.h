#pragma once

#include <set>
#include <type_traits>

#include "utils/types.h"

namespace meta::utils {

template<typename T>
concept bool Named = requires(const T& t) {
    {t.name()} -> string_view;
} || requires(T t) {
    {t->name()} -> string_view;
};

template<Named T>
struct name_comparator {
    using is_transparent = void;

    bool operator() (const T* lhs, const T* rhs) const {
        return lhs->name() < rhs->name();
    }

    bool operator() (const T& lhs, const T& rhs) const {
        return lhs.name() < rhs.name();
    }

    bool operator() (const T* lhs, string_view rhs) const {
        return lhs->name() < rhs;
    }

    bool operator() (const T& lhs, string_view rhs) const {
        return lhs.name() < rhs;
    }

    bool operator() (string_view lhs, const T* rhs) const {
        return lhs < rhs->name();
    }

    bool operator() (string_view lhs, const T& rhs) const {
        return lhs < rhs.name();
    }
};

template<Named T>
using dict = std::set<T, name_comparator<std::remove_pointer_t<T>>>;
template<Named T>
using multidict = std::multiset<T, name_comparator<std::remove_pointer_t<T>>>;

} // namespace meta::utils
