/*
 * Meta language compiler
 * Copyright (C) 2016  Sergey Vidyuk <sir.vestnik@gmail.com>
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

#include <algorithm>
#include <map>

namespace meta::utils {

using std::begin;
using std::end;

template<typename T>
concept bool Range = requires(T t) {
    {begin(t)};
    {end(t)};
    {++begin(t)};
    {begin(t) != end(t)} -> bool;
    {*begin(t)};
};

template<Range R>
struct range_traits {
    using iterator = std::decay_t<decltype(begin(std::declval<R>()))>;
    using value_type = std::decay_t<decltype(*std::declval<iterator>())>;
};

template<Range R>
using range_iterator_t = typename range_traits<R>::iterator;
template<Range R>
using range_value_t = typename range_traits<R>::value_type;

template<typename I>
concept bool Iterator = requires(I i) {
    {*i};
    {++i};
};

template<typename I, typename F>
concept bool IteratorValueTransformation =
    Iterator<I> &&
    requires(I i, F f)
{
    {f(*i)};
};

template<typename Iter, typename Func>
    requires IteratorValueTransformation<Iter, Func>
struct TransformIter {
    Func func;
    Iter it;

    bool operator!= (Iter end) {return it != end;}
    auto operator* () {return func(*it);}
    TransformIter& operator++ () {++it; return *this;}
};

template<typename Iter, typename Sentinel>
struct View {
    Iter begin_it;
    Sentinel end_it;

    Iter begin() const {return begin_it;}
    Sentinel end() const {return end_it;}
    bool empty() const {return !(begin_it != end_it);}
};

template<typename R, typename F>
    requires Range<R> && IteratorValueTransformation<range_iterator_t<R>, F>
auto operator| (R& rng, F&& func) {
    using Iter = decltype(begin(rng));
    return View<TransformIter<Iter, F>, Iter>{
        TransformIter<Iter, F>{std::forward<F>(func), begin(rng)},
        end(rng)
    };
}

template<typename Iter>
struct Slice {
    Iter begin_it;
    Iter end_it;

    Iter begin() const {return begin_it;}
    Iter end() const {return end_it;}
    bool empty() const {return !(begin_it != end_it);}
};

template<typename Iter>
auto slice(Iter b, Iter e) {return Slice<Iter>{b, e};}

template<typename Iter>
auto slice(Iter b) {Slice<Iter> res{b, b}; ++res.end_it; return res;}

template<typename Iter>
auto slice(std::pair<Iter, Iter> range) {return Slice<Iter>{range.first, range.second};}

template<typename Container, typename Key>
auto equal_range(const Container& cnt, const Key& key) {
    return slice(cnt.equal_range(key));
}

template<typename Container, typename Pred>
auto count_if(const Container& cnt, Pred&& pred) {
    return std::count_if(cnt.begin(), cnt.end(), std::forward<Pred>(pred));
}

template<typename Container, typename Pred>
auto find_if(const Container& cnt, Pred&& pred) {
    return std::find_if(cnt.begin(), cnt.end(), std::forward<Pred>(pred));
}

template<typename Container, typename BinPred>
auto adjacent_find(const Container& cnt, BinPred&& pred) {
    return std::adjacent_find(cnt.begin(), cnt.end(), std::forward<BinPred>(pred));
}

/**
 * Итератор для обхода коллекции по парам соседних элементов.
 */
template<typename Iter>
struct SegmentsIter {
    Iter head;
    Iter tailStart;

    SegmentsIter& operator++ () {
        ++head;
        ++tailStart;
        return *this;
    }

    bool operator!= (const SegmentsIter& rhs) {
        return tailStart != rhs.tailStart;
    }

    bool operator== (const SegmentsIter& rhs) {
        return tailStart == rhs.tailStart;
    }

    auto operator*() {
        return std::tie(*head, *tailStart);
    }
};

template<typename Iter>
auto segments(Iter b, Iter e) {
    if (b == e)
        return slice(SegmentsIter<Iter>{b, e}, SegmentsIter<Iter>{b, e});
    Iter n = b;
    ++n;
    return slice(SegmentsIter<Iter>{b,n}, SegmentsIter<Iter>{e,e});
}

template<typename Rng>
auto segments(const Rng& range) {
    using std::begin;
    using std::end;
    return segments(begin(range), end(range));
}

template<typename Rng>
auto segments(Rng& range) {
    using std::begin;
    using std::end;
    return segments(begin(range), end(range));
}

} // namespace meta::utils
