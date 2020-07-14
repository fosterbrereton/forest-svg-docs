/**************************************************************************************************/

// identity
#include "geometry.hpp"

// stdc++
#include <cassert>

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

point cubic_bezier::operator()(double t) const {
    assert(t >= 0);
    assert(t <= 1);

    const auto u{1 - t};
    auto p0_term{1 * std::pow(u, 3) * 1             };
    auto p1_term{3 * std::pow(u, 2) * t             };
    auto p2_term{3 * u              * std::pow(t, 2)};
    auto p3_term{1 * 1              * std::pow(t, 3)};

    return _s * p0_term + _c1 * p1_term + _c2 * p2_term + _e * p3_term;
}

/**************************************************************************************************/

point cubic_bezier::derivative(double t) const {
    assert(t >= 0);
    assert(t <= 1);

    const auto u{1 - t};
    auto p0_term{3 * std::pow(u, 2)};
    auto p1_term{6 * u * t};
    auto p2_term{3 * std::pow(t, 2)};

    return (_c1 - _s) * p0_term + (_c2 - _c1) * p1_term + (_e - _c2) * p2_term;
}

/**************************************************************************************************/

std::pair<cubic_bezier, cubic_bezier> cubic_bezier::subdivide(double t) const {
    point ab{lerp(_s, _c1, t)};
    point bc{lerp(_c1, _c2, t)};
    point cd{lerp(_c2, _e, t)};
    point abc{lerp(ab, bc, t)};
    point bcd{lerp(bc, cd, t)};
    point abcd{lerp(abc, bcd, t)};

    return std::make_pair(cubic_bezier{_s, ab, abc, abcd}, cubic_bezier{abcd, bcd, cd, _e});
}

/**************************************************************************************************/

void alp::compute_arc_table() {
    for (std::size_t i(0); i < sz_k; ++i) {
        auto t{i / static_cast<double>(sz_k)};
        _a[i] = _b(t);
    }

    _l[0] = 0;

    for (std::size_t i(0); i < (sz_k - 1); ++i) {
        const auto& a{_a[i]};
        const auto& b{_a[i + 1]};
        const auto l{(b - a).magnitude()};
        _l[i + 1] = _l[i] + l;
    }
}

/**************************************************************************************************/

double alp::find(double u) const {
    auto upper(std::upper_bound(_l.begin(), _l.end(), u));
    auto lower(upper == _l.begin() ? _l.begin() : std::prev(upper));

    assert(upper != _l.end());
    assert(lower != _l.end());

    double t_unit{1.0 / sz_k};
    auto t_first_index{std::distance(_l.begin(), lower)};
    auto t_second_index{std::distance(_l.begin(), upper)};
    double t_first{t_first_index * t_unit};
    double t_second{t_second_index * t_unit};
    double sub_t(delerp(u, *lower, *upper));

    return lerp(t_first, t_second, sub_t);
}

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/
