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
    stlab::forest<std::size_t> result;
    auto pos{result.root()};
    auto first{state._f.begin()};
    const auto last{state._f.end()};

    while (first != last) {
        ++pos;
        if (stlab::is_leading(first)) {
            auto child_count = std::distance(stlab::child_begin(first), stlab::child_end(first));
            pos = result.insert(pos, child_count);
        } else {
            pos = stlab::trailing_of(pos);
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
constexpr auto stroke_width_k{2};
constexpr auto font_size_k{16};
constexpr auto root_name_k{"&#x211C;"};

/**************************************************************************************************/

auto derive_widths(const stlab::forest<std::size_t>& counts) {
    stlab::forest<std::size_t> result;
    auto pos{result.root()};
    auto first{counts.begin()};
    const auto last{counts.end()};

    while (first != last) {
        ++pos;
        if (stlab::is_leading(first)) {
            pos = result.insert(pos, 0);
        } else {
            // now that all the children have gone, let's update our width
            if (!stlab::has_children(pos)) {
                *pos = node_size_k;
            } else {
                auto child_first{stlab::child_begin(pos)};
                auto child_last{stlab::child_end(pos)};
                while (child_first != child_last) {
                    *pos += *child_first + node_spacing_k;
                    ++child_first;
                }
                // pull out one spacing amount (it's between nodes, not one for each.)
                *pos -= node_spacing_k;
            }
            pos = stlab::trailing_of(pos);
        }
        ++first;
    }

    return result;
}

/**************************************************************************************************/

auto derive_height(const stlab::forest<std::size_t>& counts, bool leaf_edges, double margin_height) {
    stlab::depth_fullorder_iterator first{counts.begin()};
    stlab::depth_fullorder_iterator last{counts.end()};
    decltype(first.depth()) max_depth{0};

    while (first != last) {
        if (stlab::is_leading(first)) {
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

    return height + margin_height;
}

/**************************************************************************************************/

auto derive_width(const stlab::forest<std::size_t>& widths, double margin_width) {
    auto root{widths.root()};
    auto first{stlab::child_begin(root)};
    auto last{stlab::child_end(root)};
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

    return result + margin_width;
}

/**************************************************************************************************/

namespace detail {

/**************************************************************************************************/

void derive_x_offsets(stlab::forest<std::size_t>::const_iterator src,
                      stlab::forest<std::size_t>::iterator dst,
                      std::size_t parent_x_offset) {
    // It is assumed that the src and dst forest have the same shape.
    auto src_first{stlab::child_begin(src)};
    auto src_last{stlab::child_end(src)};
    auto dst_first{stlab::child_begin(dst)};

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

auto derive_x_offsets(const stlab::forest<std::size_t>& widths, double left_margin) {
    stlab::forest<std::size_t> result{widths};
    detail::derive_x_offsets(widths.root(), result.root(), left_margin);
    return result;
}

/**************************************************************************************************/

auto derive_y_offsets(const state& state, double margin_top) {
    stlab::forest<std::size_t> result;
    auto pos{result.root()};
    stlab::depth_fullorder_iterator first{state._f.begin()};
    stlab::depth_fullorder_iterator last{state._f.end()};

    while (first != last) {
        ++pos;
        if (is_leading(first)) {
            pos = result.insert(pos, margin_top + (tier_height_k + node_spacing_k) * first.depth());
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

void print_xml(stlab::forest<xml_node> xml, std::ofstream& out) {
    stlab::depth_fullorder_iterator first{xml.begin()};
    stlab::depth_fullorder_iterator last{xml.end()};

    while (first != last) {
        auto depth{first.depth()};
        auto has_content{!first->_content.empty()};
        if (is_leading(first)) {
            indent(depth, out);

            out << "<" << first->_tag;

            for (const auto& a : first->_attributes) {
                out << " " << a.first << "='" << a.second << "'";
            }

            if (stlab::has_children(first) || has_content) {
                out << ">\n";
            } else {
                out << "/>\n";
            }

            if (has_content) {
                out << first->_content << '\n';
            }
        } else if (stlab::has_children(first) || has_content) {
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

auto svg_bezier(const cubic_bezier& b) {
    return "M " + std::to_string(b._s.x) + " " + std::to_string(b._s.y) +
           " C" + std::to_string(b._c1.x) + " " + std::to_string(b._c1.y) +
           " "  + std::to_string(b._c2.x) + " " + std::to_string(b._c2.y) +
           " "  + std::to_string(b._e.x) + " " + std::to_string(b._e.y);
}

/**************************************************************************************************/

//static const point n_k{std::cos(6*M_PI_4), std::sin(6*M_PI_4)};
static const point nne_k{std::cos(6.5*M_PI_4), std::sin(6.5*M_PI_4)};
static const point ne_k{std::cos(7*M_PI_4), std::sin(7*M_PI_4)};
static const point ene_k{std::cos(7.5*M_PI_4), std::sin(7.5*M_PI_4)};
//static const point e_k{std::cos(M_PI_4), std::sin(M_PI_4)};
//static const point ese_k{std::cos(0.5*M_PI_4), std::sin(0.5*M_PI_4)};
static const point se_k{std::cos(1*M_PI_4), std::sin(1*M_PI_4)};
static const point sse_k{std::cos(1.5*M_PI_4), std::sin(1.5*M_PI_4)};
//static const point s_k{std::cos(2*M_PI_4), std::sin(2*M_PI_4)};
static const point ssw_k{std::cos(2.5*M_PI_4), std::sin(2.5*M_PI_4)};
static const point sw_k{std::cos(3*M_PI_4), std::sin(3*M_PI_4)};
//static const point wsw_k{std::cos(3.5*M_PI_4), std::sin(3.5*M_PI_4)};
//static const point w_k{std::cos(4*M_PI_4), std::sin(4*M_PI_4)};
static const point wnw_k{std::cos(4.5*M_PI_4), std::sin(4.5*M_PI_4)};
static const point nw_k{std::cos(5*M_PI_4), std::sin(5*M_PI_4)};
static const point nnw_k{std::cos(5.5*M_PI_4), std::sin(5.5*M_PI_4)};

/**************************************************************************************************/

auto edge_to_parent(const point& from, const point& to, double t, bool to_rect) {
    constexpr point node_scale_k{node_radius_k, node_radius_k};
    constexpr double min_scale_k{1.5};
    constexpr double max_scale_k{4};

    const auto out_unit{lerp(ne_k, ne_k, t)};
    const auto in_unit{lerp(se_k, se_k, t)};
    const auto scale{lerp(min_scale_k, max_scale_k, t)};

    point s;
    point c1;
    point c2;
    point e;

    if (to_rect) {
        s = from + node_scale_k * out_unit;
        c1 = from + node_scale_k * out_unit * scale;
        e = to + point{node_size_k, node_size_k * .75};
        c2 = e + point{node_spacing_k, 0};
    } else {
        s = from + out_unit * node_scale_k;
        c1 = from + out_unit * node_scale_k * scale;
        c2 = to + in_unit * node_scale_k * scale;
        e = to + in_unit * node_scale_k;
    }

    return cubic_bezier{s, c1, c2, e};
}

/**************************************************************************************************/

auto edge_to_child(const point& from, const point& to, double t, bool from_rect) {
    constexpr point node_scale_k{node_radius_k, node_radius_k};
    constexpr double min_scale_k{1.5};
    constexpr double max_scale_k{4};

    const auto out_unit{lerp(sw_k, sw_k, t)};
    const auto in_unit{lerp(nw_k, nw_k, t)};
    const auto scale{lerp(min_scale_k, max_scale_k, t)};

    point s;
    point c1;
    point c2;
    point e;

    if (from_rect) {
        s = from + point{0, node_size_k * 0.75};
        c1 = s + point{-node_spacing_k, 0};
        c2 = to + in_unit * node_scale_k * scale;
        e = to + in_unit * node_scale_k;
    } else {
        s = from + out_unit * node_scale_k;
        c1 = from + out_unit * node_scale_k * scale;
        c2 = to + in_unit * node_scale_k * scale;
        e = to + in_unit * node_scale_k;
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

    cubic_bezier result{
        from + out_unit * node_scale_k,
        from + out_unit * node_scale_k * scale,
        to + in_unit * node_scale_k * scale,
        to + in_unit * node_scale_k,
    };

    return result;
}

/**************************************************************************************************/

auto edge_to_self(const point& from, const point& to, bool rect) {
    constexpr point node_scale_k{node_radius_k, node_radius_k};
    static const auto out_unit{lerp(sw_k, ssw_k, 0.5)};
    static const auto in_unit{lerp(se_k, sse_k, 0.5)};
    cubic_bezier result;

    if (rect) {
        result._s = from + point{node_size_k / 4, node_size_k};
        result._c1 = result._s + out_unit * node_scale_k * 2;
        result._e = result._s + point{node_size_k / 2, 0};
        result._c2 = result._e + in_unit * node_scale_k * 2;
    } else {
        result._s = from + out_unit * node_scale_k;
        result._c1 = from + out_unit * node_scale_k * 3;
        result._c2 = to + in_unit * node_scale_k * 3;
        result._e = to + in_unit * node_scale_k;
    }

    return result;
}

/**************************************************************************************************/

auto edge_to_self_top(const point& root) {
    constexpr point node_scale_k{node_size_k, node_size_k};
    static const auto out_unit{lerp(ne_k, nne_k, 0.5)};
    static const auto in_unit{lerp(nw_k, nnw_k, 0.5)};
    cubic_bezier result;

    result._e = root + point{node_size_k / 4, 0};
    result._s = result._e + point{node_size_k / 2, 0};
    result._c1 = result._s + out_unit * node_scale_k;
    result._c2 = result._e + in_unit * node_scale_k;

    return result;
}

/**************************************************************************************************/

auto merge(edge_properties a, const edge_properties& b) {
    static const auto apply{[](auto& a, const auto& b, auto f){
        static const edge_properties defalt;
        const auto& fb{b.*f};
        if (fb == defalt.*f) return;
        a.*f = fb;
    }};

    apply(a, b, &edge_properties::_hide);
    apply(a, b, &edge_properties::_color);
    apply(a, b, &edge_properties::_t);
    apply(a, b, &edge_properties::_stroke_dasharray);
    apply(a, b, &edge_properties::_label_offset);
    apply(a, b, &edge_properties::_text_anchor);

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

auto derive_edges(const stlab::forest<svg::node>& f,
                  const edge_labels& labels,
                  const edge_map& map,
                  bool leaf_edges,
                  bool leading_edges,
                  bool trailing_edges,
                  bool with_root_top) {
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
    bool  prev_leading{stlab::is_leading(first)};
    const auto* prev_circle{std::get_if<svg::circle>(&*first)};
    const auto* prev_square{std::get_if<svg::square>(&*first)};

    assert(prev_circle != nullptr || prev_square != nullptr);

    bool  prev_rect{prev_square != nullptr};
    point prev{prev_rect ? prev_square->_p : prev_circle->_c};

    auto add_edge{[&_result = result](cubic_bezier bezier,
                                      const edge_properties& properties,
                                      bool cur_leading,
                                      std::string label) mutable {
        if (bezier == cubic_bezier{} || properties._hide) return;

        // get the arrowhead normal pre-trim, so it for sure points at the node.
        const auto arrowhead_normal{bezier.derivative(1).unit()};

        // trim back the bezier path to account for the arrow.
        bezier = bezier.subdivide(alp(bezier).rfind(12)).first;

        _result.push_back(svg::cubic_path{bezier,
                                          properties._color,
                                          stroke_width_k,
                                          properties._stroke_dasharray,
                                          cur_leading,
                                          std::move(label)});

        _result.push_back(svg::arrowhead{bezier._e,
                                         arrowhead_normal,
                                         properties._color});

#if 0
        // Print the control points of the curve. Save these for debugging.
        _result.push_back(svg::line{bezier._s, bezier._c1, "green", 0.5});
        _result.push_back(svg::line{bezier._e, bezier._c2, "green", 0.5});
#endif
    }};

    ++first;

    // min/max edge length delerp values
    constexpr double min_mag_k{node_size_k + node_spacing_k};
    constexpr double max_mag_k{min_mag_k * 2};

    while (first != last) {
        const bool  cur_leading{stlab::is_leading(first)};
        const auto* cur_circle{std::get_if<svg::circle>(&*first)};
        const auto* cur_square{std::get_if<svg::square>(&*first)};
        auto label{label_first != label_last ? *label_first : ""};
        edge_properties properties{derive_edge_properties(label, map, cur_leading)};

        assert(cur_circle != nullptr || cur_square != nullptr);

        bool  cur_rect{cur_square != nullptr};
        point cur{cur_rect ? cur_square->_p : cur_circle->_c};
        const auto cur_mag{(prev - cur).magnitude()};
        const auto t{delerp<double>(cur_mag, min_mag_k, max_mag_k)};
        cubic_bezier bezier;

        if (with_root_top) {
            // A special case for the root top loop. Do not advance `first` so it'll
            // get properly reused. However, we do advance the label iterator because
            // we used a label for this loop.

            add_edge(edge_to_self_top(prev),
                     derive_edge_properties(label, map, true),
                     true,
                     label);

            if (label_first != label_last) {
                ++label_first;
            }

            with_root_top = false;
            continue;
        }


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

        add_edge(bezier, properties, cur_leading, label);

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

auto derive_edge_labels(const edge_map& map, const svg::nodes& edges) {
    svg::nodes result;

    for (auto& edge : edges) {
        const auto* curve_ptr{std::get_if<svg::cubic_path>(&edge)};
        if (!curve_ptr) continue;

        const auto& curve{curve_ptr->_b};
        const auto& id{curve_ptr->_id};

        if (id.empty()) {
            continue;
        }

        auto properties{derive_edge_properties(id, map, curve_ptr->_leading)};

        assert(curve != cubic_bezier{});

        // If the edge should have been hidden, it won't even be in the edges set.
        assert(!properties._hide);

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

        const alp arcs{curve};
        const auto mid_distance{(arcs.length() + 12) / 2}; // 12 should be the same as when the edge was clipped.
        const auto midpoint_t{arcs.find(mid_distance)};
        const point mid{curve(midpoint_t)};
        const point dmid{curve.derivative(midpoint_t)};
        const double slope{dmid.x ? dmid.y / dmid.x : 0};
        const bool flat{std::abs(slope) < 0.06};
        const auto distance_k{font_size_k * properties._label_offset};

        point tp;

        if (flat) {
            tp = mid;
            tp.y -= distance_k;
        } else {
            const point pmid{point{-dmid.y, dmid.x}.unit()};

#if 0
            // Output the tangent at its magnitude. Save for debugging.
            result.push_back(svg::line{mid, mid + dmid, "blue", 0.5});
#endif
#if 0
            // Output the unit normal for this point (slightly embiggened). Save for debugging.
            result.push_back(svg::line{mid, mid + pmid * 5, "blue", 0.5});
#endif

            tp = mid + pmid * distance_k;
        }

#if 0
        // Output the point from which the label is rendered. Save this for debugging.
        result.push_back(svg::circle{tp - 0.25, 0.5, "blue", 1});
#endif

        auto split{subscript_split(id)};

        result.push_back(svg::text{
            tp,
            std::move(split.first),
            std::move(split.second),
            font_size_k,
            properties._color,
            properties._text_anchor,
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
            { "stroke-linecap", "round" },
            { "fill", "none" },
            { "stroke-width", std::to_string(path._width) },
            { "stroke-dasharray", std::move(path._stroke_dasharray) },
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
            { "text-anchor", std::move(text._text_anchor) },
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
            { "stroke-dasharray", std::move(c._stroke_dasharray) },
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

xml_node svg_to_xml(svg::arrowhead a) {
    const point perp{-a._n.y, a._n.x};
    const point p0{a._p - perp * 5};
    const point p1{a._p + a._n * 12};
    const point p2{a._p + perp * 5};

    static const auto p2s{[](const point& p){
        return std::to_string(p.x) + " " + std::to_string(p.y);
    }};

    return xml_node{
        "polygon",
        {
            { "points", p2s(p0) + ", " + p2s(p1) + ", " +p2s(p2) },
            { "fill", std::move(a._color) },
            { "stroke", "none" },
        }
    };
}

/**************************************************************************************************/

xml_node svg_to_xml(svg::node n) {
    return std::visit([](auto&& n) {
        // We call the function this way to detect at compile time if there
        // is a version of the API we want to call for the _specific_ type.
        // Don't let the compiler shoot us in the foot here by taking us to
        // some routine it thinks we want (including wrapping back around
        // into this one implicitly.)
        xml_node(*f)(std::decay_t<decltype(n)>){&svg_to_xml};
        return f(std::forward<decltype(n)>(n));
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
        auto first{stlab::child_begin(state._f.root())};
        auto last{stlab::child_end(state._f.root())};
        state._f.insert_parent(first, last, root_name_k);
    }

    auto counts = child_counts(state);
    auto widths = derive_widths(counts);
    auto height = derive_height(counts, state._s._with_leaf_edges, state._s._margin.height());
    auto width = derive_width(widths, state._s._margin.width());
    auto x_offsets = derive_x_offsets(widths, state._s._margin.l);
    auto y_offsets = derive_y_offsets(state, state._s._margin.t);

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
                stroke_width_k,
                node_properties._stroke_dasharray
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
                                !state._e["_trailing"]._hide,
                                state._s._with_root && state._s._with_root_top)};

    // Construct the node labels.

    auto svg_labels{transcribe_forest(state._f, [](const auto& n){
        auto split{subscript_split(n)};
        return svg::text{
            point{},
            std::move(split.first),
            std::move(split.second),
            font_size_k,
            "black",
            "middle"
        };
    })};

    apply_forest(svg_labels.begin(), svg_labels.end(), x_offsets.begin(), [](auto& a, auto& b){
        a._p.x = b + node_radius_k;
    });

    apply_forest(svg_labels.begin(), svg_labels.end(), y_offsets.begin(), [](auto& a, auto& b){
        a._p.y = b + node_radius_k;
    });

    // Construct edge labels

    auto edge_labels{derive_edge_labels(state._e, svg_edges)};

    // Begin constructing the final XML.

    out << "<?xml version='1.0' encoding='utf-8'?>\n";

    stlab::forest<xml_node> xml;

    // Trailing ensures the inserted elements are a child of the svg entry.
    auto p = stlab::trailing_of(xml.insert(xml.begin(), xml_node{
        "svg",
        {
            { "xmlns", "http://www.w3.org/2000/svg" },
            { "xmlns:xlink", "http://www.w3.org/1999/xlink" },
            { "width", std::to_string(width) },
            { "height", std::to_string(height) },
        }
    }));

    for (auto& edge : svg_edges) {
        xml.insert(p, svg_to_xml(std::move(edge)));
    }

    for (auto& label : edge_labels) {
        xml.insert(p, svg_to_xml(std::move(label)));
    }

    for (auto& node : stlab::preorder_range(svg_nodes)) {
        xml.insert(p, svg_to_xml(std::move(node)));
    }

    for (auto& label : stlab::preorder_range(svg_labels)) {
        xml.insert(p, svg_to_xml(std::move(label)));
    }

    print_xml(std::move(xml), out);
}

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/
