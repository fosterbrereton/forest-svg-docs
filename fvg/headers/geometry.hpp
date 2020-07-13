/**************************************************************************************************/

#ifndef FORESTVG_GEOMETRY_HPP
#define FORESTVG_GEOMETRY_HPP

/**************************************************************************************************/

#include <cmath>

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

struct point {
    double x{0};
    double y{0};

    auto magnitude() const { return std::sqrt(x * x + y * y); }

    inline constexpr auto operator+=(const point& rhs) { x += rhs.x; y += rhs.y; return *this; }
    inline constexpr auto operator-=(const point& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    inline constexpr auto operator*=(const point& rhs) { x *= rhs.x; y *= rhs.y; return *this; }
    inline constexpr auto operator/=(const point& rhs) { x /= rhs.x; y /= rhs.y; return *this; }

    inline constexpr auto operator+=(const double rhs) { x += rhs; y += rhs; return *this; }
    inline constexpr auto operator-=(const double rhs) { x -= rhs; y -= rhs; return *this; }
    inline constexpr auto operator*=(const double rhs) { x *= rhs; y *= rhs; return *this; }
    inline constexpr auto operator/=(const double rhs) { x /= rhs; y /= rhs; return *this; }
};

inline constexpr auto operator==(const point& a, const point& b) { return a.x == b.x && a.y == b.y; }
inline constexpr auto operator!=(const point& a, const point& b) { return !(a == b);}

inline constexpr auto operator+(const point& a, const point& b) { point r{a}; r += b; return r; }
inline constexpr auto operator-(const point& a, const point& b) { point r{a}; r -= b; return r; }
inline constexpr auto operator*(const point& a, const point& b) { point r{a}; r *= b; return r; }
inline constexpr auto operator/(const point& a, const point& b) { point r{a}; r /= b; return r; }

inline constexpr auto operator+(const point& a, const double b) { point r{a}; r += b; return r; }
inline constexpr auto operator-(const point& a, const double b) { point r{a}; r -= b; return r; }
inline constexpr auto operator*(const point& a, const double b) { point r{a}; r *= b; return r; }
inline constexpr auto operator/(const point& a, const double b) { point r{a}; r /= b; return r; }

inline constexpr auto operator-(const point& a) { return point{-a.x, -a.y}; }

/**************************************************************************************************/

struct cubic_bezier {
    point _s;
    point _c1;
    point _c2;
    point _e;

    point operator()(double t) const;
    point derivative(double t) const;
};

inline constexpr auto operator==(const cubic_bezier& a, const cubic_bezier& b) {
    return a._s == b._s &&
           a._c1 == b._c1 &&
           a._c2 == b._c2 &&
           a._e == b._e;
}
inline constexpr auto operator!=(const cubic_bezier& a, const cubic_bezier& b) { return !(a == b); }

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/

#endif // FORESTVG_GEOMETRY_HPP

/**************************************************************************************************/
