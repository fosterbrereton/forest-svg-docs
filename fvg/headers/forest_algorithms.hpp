/**************************************************************************************************/

#ifndef FORESTVG_FOREST_ALGORITHMS_HPP
#define FORESTVG_FOREST_ALGORITHMS_HPP

/**************************************************************************************************/

// stdc++
#include <iostream>

// stlab
#include <stlab/forest.hpp>

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

template <typename T>
void print(const stlab::forest<T>& f) {
    auto first{f.begin()};
    auto last{f.end()};
    std::size_t depth{0};

    while (first != last) {
        if (is_trailing(first)) {
            --depth;
        }

        for (std::size_t i{0}; i < depth*4; ++i) std::cout << ' ';

        if (is_leading(first)) {
            std::cout << "<";
            ++depth;
        } else {
            std::cout << "</";
        }

        std::cout << *first << ">\n";
        ++first;
    }
}

/**************************************************************************************************/

template <typename T, typename P, typename U = decltype(std::declval<P>()(T()))>
stlab::forest<U> transcribe_forest(const stlab::forest<T>& f, P&& proj) {
    stlab::forest<U> result;
    auto pos{result.root()};
    auto first{f.begin()};
    const auto last{f.end()};

    while (first != last) {
        ++pos;
        if (stlab::is_leading(first)) {
            pos = result.insert(pos, proj(*first));
        } else {
            pos = stlab::trailing_of(pos);
        }
        ++first;
    }

    return result;
}

/**************************************************************************************************/

template <typename I1, typename I2, typename P>
I2 transform_forest(I1 first, I1 last, I2 out, P&& proj) {
    while (first != last) {
        ++out;
        if (stlab::is_leading(first)) {
            out.insert(proj(*first));
        } else {
            out.trailing();
        }
        ++first;
    }

    return out;
}

/**************************************************************************************************/
// REVISIT: More closely model this after back_insert_iterator?
template <typename T>
struct forest_inserter {
    stlab::forest<T>& _f;
    typename stlab::forest<T>::iterator _p;

    explicit forest_inserter(stlab::forest<T>& f) : _f{f}, _p{_f.root()} {}

    forest_inserter& operator++() { ++_p; return *this; }

    void insert(T&& x) { _p = _f.insert(_p, std::forward<T>(x)); }

    void trailing() { _p = trailing_of(_p); }
};

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/

#endif // FORESTVG_FOREST_ALGORITHMS_HPP

/**************************************************************************************************/
