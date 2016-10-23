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
#include <iterator>
#include <tuple>

#include "utils/property.h"

namespace meta::utils {

using std::begin;
using std::end;

template<typename I>
concept bool Iterator = requires(I i) {
    {*i};
    {++i};
};

template<typename T>
concept bool Iterable = requires(T& t) {
    {begin(t)} -> Iterator;
    {end(t)};
    {begin(t) != end(t)} -> bool;
};

template<typename T, typename V>
concept bool Range = Iterable<T> && requires(T& t) {
    {*begin(t)} -> V;
};

template<Iterable R>
struct range_traits {
    using iterator = std::decay_t<decltype(begin(std::declval<R>()))>;
    using value_type = std::decay_t<decltype(*std::declval<iterator>())>;
};

template<Iterable R>
using range_iterator_t = typename range_traits<R>::iterator;
template<Iterable R>
using range_value_t = typename range_traits<R>::value_type;

template<Iterator I>
struct iterator_traits {
    using value_type = std::decay_t<decltype(*std::declval<I>())>;
};

template<Iterator I>
using iterator_value_t = typename iterator_traits<I>::value_type;

template<Iterable R, Iterator I, typename Sentinel>
struct View {
    R&& range;
    I begin_it;
    Sentinel end_it;

    auto begin() const {return begin_it;}
    Sentinel end() const {return end_it;}
    bool empty() const {return !(begin_it != end_it);}
};

template<Iterator I, typename M>
    requires MemberOf<M, iterator_value_t<I>>
struct MemberIterator {
    I iter;
    M member;

    MemberIterator& operator++ () {++iter; return *this;}
    auto& operator* () {return ((*iter).*member);}
    template<typename Sentinel>
    bool operator!= (Sentinel rhs) const {return iter != rhs;}
};

template<typename R, typename M>
    requires Iterable<R> && MemberOf<M, range_value_t<R>>
auto operator| (R&& rng, M member)
{
    using sentinel_t = decltype(end(rng));
    return View<R, MemberIterator<range_iterator_t<R>, M>, sentinel_t>{
        std::forward<R>(rng),
        MemberIterator<range_iterator_t<R>, M>{begin(rng), member},
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

template<typename Container, typename Val>
bool contains(const Container& cnt, Val&& val) {
    return std::find(cnt.begin(), cnt.end(), std::forward<Val>(val)) != cnt.end();
}

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
