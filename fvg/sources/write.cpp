/**************************************************************************************************/

// stdc++
#include <fstream>
#include <tuple>

// identity
#include "write.hpp"

// application
#include "forest_algorithms.hpp"
#include "geometry.hpp"
#include "svg.hpp"

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/
// REVISIT: Hmmm, this is transcribe_forest, but with proj(first) instead of proj(*first).
auto child_counts(const state& state) {
    adobe::forest<std::size_t> result;
    auto pos{result.root()};
    auto first{state._f.begin()};
    const auto last{state._f.end()};

    while (first != last) {
        ++pos;
        if (first.edge() == adobe::forest_leading_edge) {
            auto child_count = std::distance(adobe::child_begin(first), adobe::child_end(first));
            pos = result.insert(pos, child_count);
        } else {
            pos = adobe::trailing_of(pos);
        }
        ++first;
    }

    return result;
}

/**************************************************************************************************/

constexpr auto node_size_k{50};
constexpr auto node_radius_k{node_size_k / 2};
constexpr auto node_spacing_k{25};
constexpr auto tier_height_k{50};
constexpr auto margin_width_k{25};
constexpr auto margin_height_k{10};
constexpr auto stroke_width_k{2};
constexpr auto font_size_k{16};
constexpr auto root_name_k{"&#x211C;"};

/**************************************************************************************************/

auto derive_widths(const adobe::forest<std::size_t>& counts) {
    adobe::forest<std::size_t> result;
    auto pos{result.root()};
    auto first{counts.begin()};
    const auto last{counts.end()};

    while (first != last) {
        ++pos;
        if (first.edge() == adobe::forest_leading_edge) {
            pos = result.insert(pos, 0);
        } else {
            // now that all the children have gone, let's update our width
            if (!adobe::has_children(pos)) {
                *pos = node_size_k;
            } else {
                auto child_first{adobe::child_begin(pos)};
                auto child_last{adobe::child_end(pos)};
                while (child_first != child_last) {
                    *pos += *child_first + node_spacing_k;
                    ++child_first;
                }
                // pull out one spacing amount (it's between nodes, not one for each.)
                *pos -= node_spacing_k;
            }
            pos = adobe::trailing_of(pos);
        }
        ++first;
    }

    return result;
}

/**************************************************************************************************/

auto derive_height(const adobe::forest<std::size_t>& counts, bool leaf_edges) {
    adobe::depth_fullorder_iterator first{counts.begin()};
    adobe::depth_fullorder_iterator last{counts.end()};
    using depth_type = decltype(first.depth());
    depth_type max_depth{0};

    while (first != last) {
        if (first.edge() == adobe::forest_leading_edge) {
            max_depth = std::max(max_depth, first.depth());
        }
        ++first;
    }

    ++max_depth;

    // Keep the extra node spacing for the leaf node edges on the bottom of the graph.
    auto height{(tier_height_k + node_spacing_k) * max_depth};

    // Unless, of course, there are no leaf edges.
    if (!leaf_edges) {
        height -= node_spacing_k;
    }

    return height + margin_height_k * 2;
}

/**************************************************************************************************/

auto derive_width(const adobe::forest<std::size_t>& widths) {
    auto root{widths.root()};
    auto first{adobe::child_begin(root)};
    auto last{adobe::child_end(root)};
    auto count{0};
    auto result{0};

    while (first != last) {
        result += *first + node_spacing_k;

        ++first;
        ++count;
    }

    if (result) {
        result -= node_spacing_k;
    }

    return result + margin_width_k * 2;
}

/**************************************************************************************************/

namespace detail {

/**************************************************************************************************/

void derive_x_offsets(adobe::forest<std::size_t>::const_iterator src,
                      adobe::forest<std::size_t>::iterator dst,
                      std::size_t parent_x_offset) {
    // It is assumed that the src and dst forest have the same shape.
    auto src_first{adobe::child_begin(src)};
    auto src_last{adobe::child_end(src)};
    auto dst_first{adobe::child_begin(dst)};

    while (src_first != src_last) {
        auto old_width{*dst_first};
        *dst_first = parent_x_offset + (old_width - node_size_k) / 2;
        derive_x_offsets(src_first.base(), dst_first.base(), parent_x_offset);
        parent_x_offset += *src_first + node_spacing_k;
        ++src_first;
        ++dst_first;
    }
}

/**************************************************************************************************/

} // namespace detail

/**************************************************************************************************/

auto derive_x_offsets(const adobe::forest<std::size_t>& widths) {
    adobe::forest<std::size_t> result{widths};
    detail::derive_x_offsets(widths.root(), result.root(), margin_width_k);
    return result;
}

/**************************************************************************************************/

auto derive_y_offsets(const state& state) {
    adobe::forest<std::size_t> result;
    auto pos{result.root()};
    adobe::depth_fullorder_iterator first{state._f.begin()};
    adobe::depth_fullorder_iterator last{state._f.end()};

    while (first != last) {
        ++pos;
        if (first.edge() == adobe::forest_leading_edge) {
            pos = result.insert(pos, margin_height_k + (tier_height_k + node_spacing_k) * first.depth());
        }
        ++first;
    }

    return result;
}

/**************************************************************************************************/

struct xml_node {
    std::string _tag;
    std::unordered_map<std::string, std::string> _attributes;
    std::string _content;
};

/**************************************************************************************************/

void indent(std::size_t amount, std::ofstream& out) {
    // REVISIT: Improve performance here.
    for (std::size_t i{0}; i < amount * 4; ++i) out << ' ';
}

/**************************************************************************************************/

void print_xml(adobe::forest<xml_node> xml, std::ofstream& out) {
    adobe::depth_fullorder_iterator first{xml.begin()};
    adobe::depth_fullorder_iterator last{xml.end()};

    while (first != last) {
        auto depth{first.depth()};
        auto has_content{!first->_content.empty()};
        if (first.edge() == adobe::forest_leading_edge) {
            indent(depth, out);

            out << "<" << first->_tag;

            for (const auto& a : first->_attributes) {
                out << " " << a.first << "='" << a.second << "'";
            }

            if (adobe::has_children(first) || has_content) {
                out << ">\n";
            } else {
                out << "/>\n";
            }

            if (has_content) {
                out << first->_content << '\n';
            }
        } else if (adobe::has_children(first) || has_content) {
            indent(depth, out);

            out << "</" << first->_tag << ">\n";
        }
        ++first;
    }
}

/**************************************************************************************************/
// REVISIT: This should be the two-range variant of std::transform, though this one is inline so
// is more performant.
template <typename ForestIterator1, typename ForestIterator2, typename F>
void apply_forest(ForestIterator1 dst_first,
                  ForestIterator1 dst_last,
                  ForestIterator2 src_first,
                  F f) {
    while (dst_first != dst_last) {
        f(*dst_first, *src_first);
        ++src_first;
        ++dst_first;
    }
}

/**************************************************************************************************/

template <typename T>
auto lerp(const T& a, const T& b, double t) {
    return a + (b - a) * t;
}

template <typename T>
double delerp(const T& val, const T& lo, const T& hi) {
    return std::clamp((val - lo) / hi, 0., 1.);
}

/**************************************************************************************************/

auto svg_bezier(const cubic_bezier& b) {
    return "M " + std::to_string(b._s.x) + " " + std::to_string(b._s.y) +
           " C" + std::to_string(b._c1.x) + " " + std::to_string(b._c1.y) +
           " "  + std::to_string(b._c2.x) + " " + std::to_string(b._c2.y) +
           " "  + std::to_string(b._e.x) + " " + std::to_string(b._e.y);
}

/**************************************************************************************************/

static const point arrowhead_offset_k{9, 9};

//static const point n_k{std::cos(6*M_PI_4), std::sin(6*M_PI_4)};
static const point nne_k{std::cos(6.5*M_PI_4), std::sin(6.5*M_PI_4)};
static const point ne_k{std::cos(7*M_PI_4), std::sin(7*M_PI_4)};
static const point ene_k{std::cos(7.5*M_PI_4), std::sin(7.5*M_PI_4)};
//static const point e_k{std::cos(M_PI_4), std::sin(M_PI_4)};
static const point ese_k{std::cos(0.5*M_PI_4), std::sin(0.5*M_PI_4)};
static const point se_k{std::cos(1*M_PI_4), std::sin(1*M_PI_4)};
static const point sse_k{std::cos(1.5*M_PI_4), std::sin(1.5*M_PI_4)};
//static const point s_k{std::cos(2*M_PI_4), std::sin(2*M_PI_4)};
static const point ssw_k{std::cos(2.5*M_PI_4), std::sin(2.5*M_PI_4)};
static const point sw_k{std::cos(3*M_PI_4), std::sin(3*M_PI_4)};
static const point wsw_k{std::cos(3.5*M_PI_4), std::sin(3.5*M_PI_4)};
//static const point w_k{std::cos(4*M_PI_4), std::sin(4*M_PI_4)};
static const point wnw_k{std::cos(4.5*M_PI_4), std::sin(4.5*M_PI_4)};
static const point nw_k{std::cos(5*M_PI_4), std::sin(5*M_PI_4)};
static const point nnw_k{std::cos(5.5*M_PI_4), std::sin(5.5*M_PI_4)};

/**************************************************************************************************/

auto edge_to_parent(const point& from, const point& to, double t, bool to_rect) {
    constexpr point node_scale_k{node_radius_k, node_radius_k};
    constexpr double min_scale_k{1.5};
    constexpr double max_scale_k{2.5};

    const auto out_unit{lerp(ne_k, nne_k, t)};
    const auto in_unit{lerp(se_k, ese_k, t)};
    const auto scale{lerp(min_scale_k, max_scale_k, t)};

    point s;
    point c1;
    point c2;
    point e;

    if (to_rect) {
        s = from + node_scale_k * out_unit;
        c1 = from + node_scale_k * out_unit * scale;
        e = to + point{node_size_k, node_size_k * .75} + arrowhead_offset_k;
        c2 = e + point{margin_width_k, margin_width_k};
    } else {
        const auto dst{to + arrowhead_offset_k};
        s = from + out_unit * node_scale_k;
        c1 = from + out_unit * node_scale_k * scale;
        c2 = dst + in_unit * node_scale_k * scale;
        e = dst + in_unit * node_scale_k;
    }

    return cubic_bezier{s, c1, c2, e};
}

/**************************************************************************************************/

auto edge_to_child(const point& from, const point& to, double t, bool from_rect) {
    constexpr point node_scale_k{node_radius_k, node_radius_k};
    constexpr double min_scale_k{1.5};
    constexpr double max_scale_k{2.5};

    const auto out_unit{lerp(sw_k, wsw_k, t)};
    const auto in_unit{lerp(nw_k, nnw_k, t)};
    const auto scale{lerp(min_scale_k, max_scale_k, t)};
    const auto dst{to - arrowhead_offset_k};

    point s;
    point c1;
    point c2;
    point e;

    if (from_rect) {
        s = from + point{0, node_size_k * 0.75};
        c1 = s + point{-margin_width_k, margin_width_k};
        c2 = dst + in_unit * node_scale_k * scale;
        e = dst + in_unit * node_scale_k;
    } else {
        s = from + out_unit * node_scale_k;
        c1 = from + out_unit * node_scale_k * scale;
        c2 = dst + in_unit * node_scale_k * scale;
        e = dst + in_unit * node_scale_k;
    }

    return cubic_bezier{s, c1, c2, e};
}

/**************************************************************************************************/

auto edge_to_sibling(const point& from, const point& to, double t) {
    constexpr point node_scale_k{node_radius_k, node_radius_k};
    constexpr double min_scale_k{1.5};
    constexpr double max_scale_k{2.5};

    const auto out_unit{lerp(ne_k, ene_k, t)};
    const auto in_unit{lerp(nw_k, wnw_k, t)};
    const auto scale{lerp(min_scale_k, max_scale_k, t)};
    const auto dst{to - arrowhead_offset_k};

    point s = from + out_unit * node_scale_k;
    point c1 = from + out_unit * node_scale_k * scale;
    point c2 = dst + in_unit * node_scale_k * scale;
    point e = dst + in_unit * node_scale_k;

    return cubic_bezier{s, c1, c2, e};
}

/**************************************************************************************************/

auto edge_to_self(const point& from, const point& to, bool rect) {
    constexpr point node_scale_k{node_radius_k, node_radius_k};
    const auto out_unit{ssw_k};
    const auto in_unit{sse_k};

    point s;
    point c1;
    point c2;
    point e;

    if (rect) {
        s = from + point{node_size_k / 4, node_size_k};
        c1 = s + point{-margin_width_k, margin_width_k};
        e = s + point{node_size_k / 2, 0} + point{0, 9};
        c2 = e + point{margin_width_k, margin_width_k};
    } else {
        const auto dst{to + arrowhead_offset_k};
        s = from + out_unit * node_scale_k;
        c1 = from + out_unit * node_scale_k * 2.3;
        c2 = dst + in_unit * node_scale_k * 1.5;
        e = dst + in_unit * node_scale_k;
    }

    return cubic_bezier{s, c1, c2, e};
}

/**************************************************************************************************/

auto merge(edge_properties a, const edge_properties& b) {
    const edge_properties dfault;

    // If b's value isn't a default value, it wins over whatever value was in a.

    if (b._hide != dfault._hide) {
        a._hide = b._hide;
    }

    if (b._color != dfault._color) {
        a._color = b._color;
    }

    return a;
}

/**************************************************************************************************/

auto derive_edge_properties(const std::string& edge_name, const edge_map& map, bool leading) {
    const auto pfound{map.find(edge_name)};
    edge_properties result{pfound != map.end() ? pfound->second : edge_properties()};

    const auto& edge_properties_name{leading ? "_leading" : "_trailing"};

    const auto efound{map.find(edge_properties_name)};
    if (efound != map.end()) {
        result = merge(efound->second, result);
    }

    return result;
}

/**************************************************************************************************/

auto derive_edges(const adobe::forest<svg::node>& f,
                  const edge_labels& labels,
                  const edge_map& map,
                  bool leaf_edges,
                  bool leading_edges,
                  bool trailing_edges) {
    // The goal is to have the lines look as natural as possible. This should include the arrowhead
    // at the end of the line. SVG allows you to attach an arrowhead at the end of the line via the
    // marker attribute. The bad news is this arrowhead's base is at the end of the line, and the
    // addition of the arrowhead causes the line+arrowhead combination to terminate within the
    // bounds of the node. I tried using the refX and refY attributes of the marker to set the
    // arrowhead back by its own width (so it would finish at the same point the line does) but the
    // problem with that is the arrowhead's angle is set to match the end of the line, not the angle
    // where it is inset to, causing the arrow to look wonky on longer/more extremely angled edges.
    // The next fix is to stop the line an arrowhead's width before reaching the node, and allowing
    // the arrowhead to complete the line correctly. The problem there is now one of symmetry: with
    // the start of the bezier path being on a node boundary, and the end being prior to a node
    // boundary, symmetrical control points do not yield a symmetrical bezier path. The current
    // solution I have for that problem is to add a "stub line" to the start of the path that is the
    // same length as an arrowhead. While this can look a bit strange for longer/more extremely
    // angled lines, it does achieve the desired bezier symmetry, as well as helping to minimize
    // edges colliding with other graph elements. No doubt I could solve the same problem by
    // using non-symmetrical control points, except the bezier math gets crazy.
    //
    // Another improvement I can make is to lessen the angle of departure/approach for long edges.
    // (i.e., not requiring the edges start exactly at NW, NE, SW, and SE.)
    // The long ones have the most extreme angles and so the arrowheads / stubs can look pretty
    // funny. Another alternative solution here is have a larger scale for the control points in
    // these cases, or have the control point scale between two values depending on the length
    // of the edge (some kind of linear interpolation.)
    //
    // In the end, symmetry isn't worth it. The code is much cleaner if there's a single
    // cubic path for each edge in the graph. So that's where we'll go. I did implement the
    // extreme/long edge angle backoff- that seems to have helped a bit.

    svg::nodes result;

    if (f.empty()) return result;

    auto  first{f.begin()};
    auto  last{f.end()};
    auto  label_first{labels.begin()};
    auto  label_last{labels.end()};
    bool  prev_leading{first.edge() == adobe::forest_leading_edge};
    const auto* prev_circle{std::get_if<svg::circle>(&*first)};
    const auto* prev_square{std::get_if<svg::square>(&*first)};

    assert(prev_circle != nullptr || prev_square != nullptr);

    bool  prev_rect{prev_square != nullptr};
    point prev{prev_rect ? prev_square->_p : prev_circle->_c};

    ++first;

    // min/max edge length delerp values
    constexpr double min_mag_k{node_size_k + node_spacing_k};
    constexpr double max_mag_k{min_mag_k * 2};

    while (first != last) {
        bool  cur_leading{first.edge() == adobe::forest_leading_edge};
        const auto* cur_circle{std::get_if<svg::circle>(&*first)};
        const auto* cur_square{std::get_if<svg::square>(&*first)};
        edge_properties properties;

        if (label_first != label_last) {
            properties = derive_edge_properties(*label_first, map, cur_leading);
        }

        assert(cur_circle != nullptr || cur_square != nullptr);

        bool  cur_rect{cur_square != nullptr};
        point cur{cur_rect ? cur_square->_p : cur_circle->_c};
        const auto t{delerp<double>((prev - cur).magnitude(), min_mag_k, max_mag_k)};
        cubic_bezier bezier;

        if (prev_leading) {
            if (cur_leading && leading_edges) {
                bezier = edge_to_child(prev, cur, t, prev_rect);
            } else if (leaf_edges) {
                bezier = edge_to_self(prev, cur, prev_rect);
            }
        } else {
            if (cur_leading && leading_edges) {
                bezier = edge_to_sibling(prev, cur, t);
            } else if (trailing_edges){
                bezier = edge_to_parent(prev, cur, t, cur_rect);
            }
        }

        if (bezier != cubic_bezier{}) {
            result.push_back(svg::cubic_path{bezier, std::move(properties._color), stroke_width_k, cur_leading});

#if 0
            // Print the control points of the curve. Save these for debugging.
            result.push_back(svg::line{bezier._s, bezier._c1, "green", 2});
            result.push_back(svg::line{bezier._e, bezier._c2, "green", 2});
#endif
        }

        prev_rect = cur_rect;
        prev_leading = cur_leading;
        prev = cur;

        ++first;

        if (label_first != label_last) {
            ++label_first;
        }
    }

    return result;
}

/**************************************************************************************************/

auto subscript_split(const std::string& s) {
    auto subscript_pos{s.find("_")};

    if (subscript_pos == std::string::npos) return std::make_pair(s, std::string());

    std::string pre{s.substr(0, subscript_pos)};
    std::string sub{s.substr(subscript_pos + 1)};

    return std::make_pair(std::move(pre), std::move(sub));
}

/**************************************************************************************************/

auto derive_edge_labels(const edge_labels& labels, const edge_map& map, const svg::nodes& edges) {
    auto       first{labels.begin()};
    auto       last{labels.end()};
    svg::nodes result;

    for (auto& edge : edges) {
        if (first == last) break;
        if (first->empty()) {
            ++first;
            continue;
        }

        const auto* curve_ptr{std::get_if<svg::cubic_path>(&edge)};
        if (!curve_ptr) continue;
        const auto& curve{curve_ptr->_b};
        auto properties{derive_edge_properties(*first, map, curve_ptr->_leading)};

        assert(curve != cubic_bezier{});

        if (properties._hide) {
            ++first;
            continue;
        }

        // The goal is to put the label along the edge. To do that we have to compute
        // a point along the curve where we want to put the label, then an offset from
        // that point where we want the label to go (lest the label be directly over
        // the edge.) To get the offset, we take the derivative of the bezier at the
        // same point, and compute its slope. Inverting that slope gives us a line
        // that's perpendicular to the curve at the point we care about, and the label
        // can "slide" up and down that line to figure out where we want it to go. For
        // curves that are more or less flat at the point where we want the label (for
        // some definition of "more or less flat"), we take an alternative path that
        // does not rely on the slope of the perpendicular.

        const point mid{curve(0.5)};
        const point dmid{curve.derivative(0.5)};
        const double slope{dmid.x ? dmid.y / dmid.x : 0};
        const bool flat{std::abs(slope) < 0.01};
        constexpr auto distance_k{font_size_k * 0.55};

        point bs; // baseline start
        point be; // baseline end

        if (flat) {
            bs = mid;
            bs.y += distance_k;
            be = mid;
            be.y -= distance_k;
        } else {
            const double orth{-1 / slope};
            const auto b{mid.y - orth * mid.x};

            // The goal here is to get two points that are a certain distance from the computed
            // bezier point along the line that is perpendicular to the tanget at that point in the
            // bezier. Blah, that was a mouthful. No time to sum up, let me explain. We're adding a
            // label to an edge, which is drawn via a bezier path. In order for viewers to associate
            // the label to the path, that label needs to be a certain distance from the path, but
            // related to a point along the path. For ease of explaining, let's say that point is t
            // = 0.5, the bezier "midpoint". We want the label some distance away from the curve
            // relative to that "midpoint". That requires the label move perpendicular to the bezier
            // curve at that point. That requires getting the tangent first, and inverting the
            // slope.

            // To get these two points some distance away from but perpendicular to the bezier path
            // at a certain point, we compute one of the points and then can derive the other. To
            // compute the one point, we have two unknowns, x, and y. Therefore, we need two
            // equations, solve for y in terms of x in one, and then replace y in the other equation
            // and solve for x. The first equation I used was the distance equation:

            // d^2 = (x2 - x1)^2 + (y2 - y1)^2

            // and the second was the two-point line equation:

            // (y2 - y1) = m * (x2 - x1)

            // Above, all the values are constants except x2 and y2. (x1, y1) is the bezier
            // "midpoint", d is a fixed distance along the perpendicular that we're hunting for, and
            // m is the slope of the perpendicular at the "midpoint". Using those two formulas, I
            // was able to get a solution to the point where the quadratic equation could be used to
            // solve for x. Given a quadratic ax^2 + bx + c = 0, the constants for my situation are:

            // a = -1
            // b = 2 * mid.x
            // c = -(mid.x^2 * (1 + m^2) - d^2) / (1 + m^2)

            // The added benefit of the quadratic formula is that it gives me both x values that are
            // equidistant along the perpendicular at the bezier "midpoint". Once I had x for both
            // points, I used the slope-intercept line equation to get their corresponding y values.

            const auto d2{distance_k * distance_k};
            const auto qa{-1};
            const auto qb{2 * mid.x};
            const auto o2{orth * orth};
            const auto qc_num{std::pow(mid.x, 2) * (1 + o2) - d2};
            const auto qc_den{1 + o2};
            const auto qc{-qc_num / qc_den};
            const auto qs{std::sqrt(std::pow(qb, 2) - 4 * qa * qc)};
            const auto x1{(-qb + qs) / (2 * qa)};
            const auto x2{(-qb - qs) / (2 * qa)};

            bs.x = x1;
            be.x = x2;
            bs.y = orth * bs.x + b;
            be.y = orth * be.x + b;

            // we want bs to be the higher of the two values, so the label always
            // shows above the edge.
            if (bs.y > be.y) {
                std::swap(bs, be);
            }
        }

#if 0
        // Output the perpendicular line upon which the label rests. Save this for debugging.
        result.push_back(svg::line{bs, be, "yellow", 2});
#endif

        auto split{subscript_split(*first++)};

        result.push_back(svg::text{
            bs,
            std::move(split.first),
            std::move(split.second),
            font_size_k,
            properties._color,
        });
    }

    return result;
}

/**************************************************************************************************/

xml_node svg_to_xml(svg::line line) {
    return xml_node{
        "line",
        {
            { "x1", std::to_string(line._a.x) },
            { "y1", std::to_string(line._a.y) },
            { "x2", std::to_string(line._b.x) },
            { "y2", std::to_string(line._b.y) },
            { "stroke", std::move(line._color) },
            { "stroke-width", std::to_string(line._width) },
        }
    };
}

/**************************************************************************************************/

xml_node svg_to_xml(svg::cubic_path path) {
    return xml_node{
        "path",
        {
            { "d", svg_bezier(path._b) },
            { "stroke", std::move(path._color) },
            { "fill", "none" },
            { "stroke-width", std::to_string(path._width) },
            { "marker-end", "url(#arrowhead)" },
        }
    };
}

/**************************************************************************************************/

xml_node svg_to_xml(svg::text text) {
    const auto drop{std::to_string(text._size / 2)};

    const auto string{std::move(text._s) +
           "<tspan dy=\'" + drop + "\' font-size=\'.7em\'>" +
           std::move(text._subscript) +
           "</tspan>"
    };

    return xml_node{
        "text",
        {
            { "font-size", std::to_string(text._size) },
            { "text-anchor", "middle" },
            { "dominant-baseline", "central" },
            { "x", std::to_string(text._p.x) },
            { "y", std::to_string(text._p.y) },
            { "fill", std::move(text._color) }
        },
        std::move(string)
    };
}

/**************************************************************************************************/

xml_node svg_to_xml(svg::circle c) {
    return xml_node{
        "circle",
        {
            { "cx", std::to_string(c._c.x) },
            { "cy", std::to_string(c._c.y) },
            { "r", std::to_string(c._r) },
            { "fill", "white" },
            { "stroke", std::move(c._color) },
            { "stroke-width", std::to_string(c._stroke_width) },
            //{ "stroke-dasharray", node_properties._stroke_dasharray },
        }
    };
}

/**************************************************************************************************/

xml_node svg_to_xml(svg::square s) {
    return xml_node{
        "rect",
        {
            { "x", std::to_string(s._p.x) },
            { "y", std::to_string(s._p.y) },
            { "width", std::to_string(s._size) },
            { "height", std::to_string(s._size) },
            { "fill", "white" },
            { "stroke", std::move(s._color) },
            { "stroke-width", std::to_string(s._stroke_width) },
        }
    };
}

/**************************************************************************************************/

xml_node svg_to_xml(svg::node n) {
    return std::visit([](auto&& n) {
        return svg_to_xml(std::forward<decltype(n)>(n));
    }, std::move(n));
}

/**************************************************************************************************/

void write_svg(state state, const std::filesystem::path& path) {
    std::ofstream out{path, std::ios::out | std::ios::binary};

    if (!out) {
        throw std::runtime_error("error creating output file");
    }

    // Add optional root
    if (state._s._with_root) {
        auto first{adobe::child_begin(state._f.root())};
        auto last{adobe::child_end(state._f.root())};
        state._f.insert_parent(first, last, root_name_k);
    }

    auto counts = child_counts(state);
    auto widths = derive_widths(counts);
    auto height = derive_height(counts, state._s._with_leaf_edges);
    auto width = derive_width(widths);
    auto x_offsets = derive_x_offsets(widths);
    auto y_offsets = derive_y_offsets(state);

    // Save for debugging.
    // fvg::print(state._f);
    // std::cout << "=-=-=-=-\n";
    // fvg::print(counts);
    // std::cout << "=-=-=-=-\n";
    // fvg::print(widths);
    // std::cout << "=-=-=-=-\n";
    // fvg::print(x_offsets);
    // std::cout << "=-=-=-=-\n";
    // fvg::print(y_offsets);

    // Construct the nodes.

    auto svg_nodes{transcribe_forest(state._f, [&_map = state._n](const auto& n){
        const auto& node_properties{_map[n]};
        return n == root_name_k ?
            svg::node{svg::square{
                point{},
                node_size_k,
                "darkred",
                stroke_width_k
            }} :
            svg::node{svg::circle{
                point{},
                node_radius_k,
                node_properties._color,
                stroke_width_k
            }};
    })};

    apply_forest(svg_nodes.begin(), svg_nodes.end(), x_offsets.begin(), [](auto& a, auto& b){
        if (auto* circle = std::get_if<svg::circle>(&a)) {
            circle->_c.x = b + node_radius_k;
        } else if (auto* square = std::get_if<svg::square>(&a)) {
            square->_p.x = b;
        } else {
            throw std::runtime_error("Unknown node shape");
        }
    });

    apply_forest(svg_nodes.begin(), svg_nodes.end(), y_offsets.begin(), [](auto& a, auto& b){
        if (auto* circle = std::get_if<svg::circle>(&a)) {
            circle->_c.y = b + node_radius_k;
        } else if (auto* square = std::get_if<svg::square>(&a)) {
            square->_p.y = b;
        } else {
            throw std::runtime_error("Unknown node shape");
        }
    });

    // Derive the edges.

    auto svg_edges{derive_edges(svg_nodes,
                                state._l,
                                state._e,
                                state._s._with_leaf_edges,
                                !state._e["_leading"]._hide,
                                !state._e["_trailing"]._hide)};

    // Construct the node labels.

    auto svg_labels{transcribe_forest(state._f, [](const auto& n){
        auto split{subscript_split(n)};
        return svg::text{
            point{},
            std::move(split.first),
            std::move(split.second),
            font_size_k
        };
    })};

    apply_forest(svg_labels.begin(), svg_labels.end(), x_offsets.begin(), [](auto& a, auto& b){
        a._p.x = b + node_radius_k;
    });

    apply_forest(svg_labels.begin(), svg_labels.end(), y_offsets.begin(), [](auto& a, auto& b){
        a._p.y = b + node_radius_k;
    });

    // Construct edge labels

    auto edge_labels{derive_edge_labels(state._l, state._e, svg_edges)};

    // Begin constructing the final XML.

    out << "<?xml version='1.0' encoding='utf-8'?>\n";

    adobe::forest<xml_node> xml;

    // Trailing ensures the inserted elements are a child of the svg entry.
    auto p = adobe::trailing_of(xml.insert(xml.begin(), xml_node{
        "svg",
        {
            { "xmlns", "http://www.w3.org/2000/svg" },
            { "xmlns:xlink", "http://www.w3.org/1999/xlink" },
            { "width", std::to_string(width) },
            { "height", std::to_string(height) },
        }
    }));

    auto defs = adobe::trailing_of(xml.insert(p, xml_node{ "defs" }));
    auto marker_arrowhead = adobe::trailing_of(xml.insert(defs, xml_node{
        "marker",
        {
            { "id", "arrowhead" },
            { "markerWidth", "12" },
            { "markerHeight", "10" },
            { "refY", "5" },
            { "orient", "auto-start-reverse" },
            { "markerUnits", "userSpaceOnUse" },
        }
    }));
    xml.insert(marker_arrowhead, xml_node{
        "polygon",
        {
            { "points", "0 0, 12 5, 0 10" },
            // Apparently these are an SVG 2.0 feature that aren't supported well... anywhere.
            // { "fill", "context-stroke" },
            // { "stroke", "context-stroke" },
        }
    });

    for (auto& edge : svg_edges) {
        xml.insert(p, svg_to_xml(std::move(edge)));
    }

    for (auto& label : edge_labels) {
        xml.insert(p, svg_to_xml(std::move(label)));
    }

    for (auto& node : adobe::preorder_range(svg_nodes)) {
        xml.insert(p, svg_to_xml(std::move(node)));
    }

    for (auto& label : adobe::preorder_range(svg_labels)) {
        xml.insert(p, svg_to_xml(std::move(label)));
    }

    print_xml(std::move(xml), out);
}

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/
