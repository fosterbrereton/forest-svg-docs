/**************************************************************************************************/

#ifndef FORESTVG_SVG_HPP
#define FORESTVG_SVG_HPP

/**************************************************************************************************/

// stdc++
#include <string>
#include <variant>
#include <vector>

// application
#include "geometry.hpp"

/**************************************************************************************************/

namespace fvg {
namespace svg {

/**************************************************************************************************/

struct line {
    point _a;
    point _b;
    std::string _color;
    double _width;
};

/**************************************************************************************************/

struct cubic_path {
    cubic_bezier _b;
    std::string _color;
    double _width;
    bool _leading; // a forest-specific detail
};

/**************************************************************************************************/

struct text {
    point _p; // baseline midpoint (for now)
    std::string _s;
    std::string _subscript;
    double _size;
    std::string _color;
};

/**************************************************************************************************/

struct circle {
    point _c; // centerpoint
    double _r;
    std::string _color;
    double _stroke_width;
    // dasharray
};

/**************************************************************************************************/

struct square {
    point _p; // top-left
    double _size;
    std::string _color;
    double _stroke_width;
};

/**************************************************************************************************/

using node = std::variant<line, cubic_path, text, circle, square>;
using nodes = std::vector<node>;

/**************************************************************************************************/

} // namespace svg
} // namespace fvg

/**************************************************************************************************/

#endif // FORESTVG_SVG_HPP

/**************************************************************************************************/
