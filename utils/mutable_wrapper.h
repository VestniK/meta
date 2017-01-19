#pragma once

#include <utility>

namespace meta::utils {

template<typename T>
class mutable_wrapper {
public:
    template<typename... A>
    mutable_wrapper(A&&... a): mVal(std::forward<A>(a)...) {}

    T& get() const {return mVal;}

    T& operator* () const {return mVal;}
    T* operator-> () const {return &mVal;}

private:
    mutable T mVal;
};

} // namespace meta::utils
