#pragma once

#include <type_traits>

namespace meta::utils {

template<typename T>
concept bool NonVoid = !std::is_void<T>::value;

template<typename T, class C>
concept bool MemberOf = requires(T t, C& c) {
    {(c.*t)};
};

template<typename T, class C>
concept bool MethodOf = MemberOf<T, C> && std::is_member_function_pointer<T>::value;

template<typename T, class C>
concept bool FieldOf = MemberOf<T, C> && !std::is_member_object_pointer<T>::value;

}
