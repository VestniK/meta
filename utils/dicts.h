#pragma once

#include <set>

#include "utils/types.h"

namespace meta::utils {

template<typename T>
concept bool Named = requires(const T& t) {
    {t.name()} -> string_view;
} || requires(T t) {
    {t->name()} -> string_view;
};

namespace detail {

auto name(const Named& val) -> decltype(val.name()) {return val.name();}
auto name(const Named& val) -> decltype(val->name()) {return val->name();}

} // namespace detail

template<Named T>
struct name_comparator {
    using is_transparent = void;

    bool operator() (const T& lhs, const T& rhs) const {
        return detail::name(lhs) < detail::name(rhs);
    }

    bool operator() (const T& lhs, string_view rhs) const {
        return detail::name(lhs) < rhs;
    }

    bool operator() (string_view lhs, const T& rhs) const {
        return lhs < detail::name(rhs);
    }
};

template<Named T>
using dict = std::set<T, name_comparator<T>>;
template<Named T>
using multidict = std::multiset<T, name_comparator<T>>;

} // namespace meta::utils
