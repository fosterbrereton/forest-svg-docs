/**************************************************************************************************/

#ifndef FORESTVG_GEOMETRY_HPP
#define FORESTVG_GEOMETRY_HPP

/**************************************************************************************************/

#include <algorithm>
#include <array>
#include <cmath>

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

template <typename T>
auto lerp(const T& a, const T& b, double t) {
    return a + (b - a) * t;
}

template <typename T>
double delerp(const T& val, const T& lo, const T& hi) {
    return std::clamp((val - lo) / (hi - lo), 0., 1.);
}

/**************************************************************************************************/

struct point {
    double x{0};
    double y{0};

    auto magnitude() const { return std::sqrt(x * x + y * y); }

    auto unit() const;

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

inline auto point::unit() const { return *this / magnitude(); }

/**************************************************************************************************/

// for things like margins, padding, etc. Where you're not defining a box per se.
struct extents {
    double l;
    double t;
    double r;
    double b;

    auto width() const { return l + r; }
    auto height() const { return t + b; }
};

/**************************************************************************************************/

struct cubic_bezier {
    point _s;
    point _c1;
    point _c2;
    point _e;

    point operator()(double t) const;
    point derivative(double t) const;
    std::pair<cubic_bezier, cubic_bezier> subdivide(double t) const;
};

inline constexpr auto operator==(const cubic_bezier& a, const cubic_bezier& b) {
    return a._s == b._s &&
           a._c1 == b._c1 &&
           a._c2 == b._c2 &&
           a._e == b._e;
}
inline constexpr auto operator!=(const cubic_bezier& a, const cubic_bezier& b) { return !(a == b); }

/**************************************************************************************************/
// arc-length parameterization
struct alp {
    explicit alp(cubic_bezier b) : _b(std::move(b)) {
        compute_arc_table();
    }

    double length() const { return _l.back(); }

    double find(double u) const;
    double rfind(double u) const { return find(length() - u); }

private:
    static constexpr auto sz_k{128};

    void compute_arc_table();

    const cubic_bezier _b;
    std::array<point, sz_k> _a; // points
    std::array<double, sz_k> _l; // accumulated lengths
};

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/

#endif // FORESTVG_GEOMETRY_HPP

/**************************************************************************************************/
