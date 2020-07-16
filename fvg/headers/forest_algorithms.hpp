/**************************************************************************************************/

#ifndef FORESTVG_FOREST_ALGORITHMS_HPP
#define FORESTVG_FOREST_ALGORITHMS_HPP

/**************************************************************************************************/

#include <iostream>

#include "forest_fwd.hpp"

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

template <typename T>
void print(const forest<T>& f) {
    auto first{f.begin()};
    auto last{f.end()};
    std::size_t depth{0};

    while (first != last) {
        if (first.edge() == FNS::forest_trailing_edge) {
            --depth;
        }
        for (std::size_t i{0}; i < depth*4; ++i) std::cout << ' ';
        if (first.edge() == FNS::forest_leading_edge) {
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
forest<U> transcribe_forest(const forest<T>& f, P&& proj) {
    forest<U> result;
    auto pos{result.root()};
    auto first{f.begin()};
    const auto last{f.end()};

    while (first != last) {
        ++pos;
        if (first.edge() == FNS::forest_leading_edge) {
            pos = result.insert(pos, proj(*first));
        } else {
            pos = FNS::trailing_of(pos);
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
        if (first.edge() == FNS::forest_leading_edge) {
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
    forest<T>& _f;
    typename forest<T>::iterator _p;

    explicit forest_inserter(forest<T>& f) : _f{f}, _p{_f.root()} {}

    forest_inserter& operator++() { ++_p; return *this; }

    void insert(T&& x) { _p = _f.insert(_p, std::forward<T>(x)); }

    void trailing() { _p = trailing_of(_p); }
};

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/

#endif // FORESTVG_FOREST_ALGORITHMS_HPP

/**************************************************************************************************/
