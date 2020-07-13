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

} // namespace fvg

/**************************************************************************************************/
