/**************************************************************************************************/

// stdc++
#include <fstream>

// identity
#include "write.hpp"

// application
#include "forest_algorithms.hpp"

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
constexpr auto margin_k{25};
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

auto derive_height(const adobe::forest<std::size_t>& counts) {
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

    return height + margin_k * 2;
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

    return result + margin_k * 2;
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
    detail::derive_x_offsets(widths.root(), result.root(), margin_k);
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
            pos = result.insert(pos, margin_k + (tier_height_k + node_spacing_k) * first.depth());
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

    // Not necessary for XML output, but handy to keep around.
    double _x{0};
    double _y{0};
    double _w{0};
    double _h{0};
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

inline constexpr auto operator==(const point& a, const point& b) { return a.x == b.x && a.y == b.y;}
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

template <typename T>
auto lerp(const T& a, const T& b, double t) {
    return a + (b - a) * t;
}

/**************************************************************************************************/

auto make_cubic_curve(const point& start, const point& control_1, const point& control_2, const point& end) {
    return "M " + std::to_string(start.x) + " " + std::to_string(start.y) +
           " C" + std::to_string(control_1.x) + " " + std::to_string(control_1.y) +
           " "  + std::to_string(control_2.x) + " " + std::to_string(control_2.y) +
           " "  + std::to_string(end.x) + " " + std::to_string(end.y);
}

/**************************************************************************************************/

auto derive_edges(const adobe::forest<xml_node>& f) {
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

    std::vector<xml_node> result;

    if (f.empty()) return result;

    auto        first{f.begin()};
    auto        last{f.end()};
    bool        prev_leading{first.edge() == adobe::forest_leading_edge};
    bool        prev_rect{first->_tag == "rect"};
    point       prev{first->_x, first->_y};
    std::size_t edge_count{0};

    ++first;

    constexpr point arrowhead_offset_k{12, 12};
    constexpr point node_scale_k{node_radius_k, node_radius_k};
    constexpr point out_scale_k{node_scale_k};
    constexpr point out_stub_k{node_scale_k + arrowhead_offset_k};
    constexpr point in_scale_k{arrowhead_offset_k + node_radius_k};
    static const point se_in_k{point{std::cos(1*M_PI_4), std::sin(1*M_PI_4)} * in_scale_k};
    static const point nw_in_k{point{std::cos(5*M_PI_4), std::sin(5*M_PI_4)} * in_scale_k};

    while (first != last) {
        bool  cur_leading{first.edge() == adobe::forest_leading_edge};
        bool  cur_rect{first->_tag == "rect"};
        point cur{first->_x, first->_y};
        point stub_start;
        point s;
        point c1;
        point c2;
        point e;

        if (prev_leading) {
            static const point sw_k{point{std::cos(3*M_PI_4), std::sin(3*M_PI_4)}};
            static const point sw_out_k{sw_k * out_scale_k};
            static const point sw_stub_out_k{sw_k * out_stub_k};

            if (cur_leading) {
                // down to first child
                const point c_scale_k{1.25, 1.25};
                if (!prev_rect) {
                    stub_start = prev + sw_out_k;
                    s = prev + sw_stub_out_k;
                    c1 = prev + sw_stub_out_k * c_scale_k;
                    c2 = cur + nw_in_k * c_scale_k;
                    e = cur + nw_in_k;
                } else {
                    s = prev;
                    s.y += node_size_k / 2;
                    stub_start = s;
                    s.x -= arrowhead_offset_k.x;
                    c1 = prev + sw_out_k * c_scale_k;
                    c2 = cur + nw_in_k * c_scale_k;
                    e = cur + nw_in_k;
                }
            } else {
                // leaf node loop
                const point c_scale_k{2.5, 2.5};
                stub_start = prev + sw_out_k;
                s = prev + sw_stub_out_k;
                c1 = prev + sw_stub_out_k * c_scale_k;
                c2 = cur + se_in_k * c_scale_k;
                e = cur + se_in_k;
            }
        } else {
#if 0
            static const point ne_min_k{std::cos(7*M_PI_4), std::sin(7*M_PI_4)};
            static const point ne_max_k{std::cos(6.5*M_PI_4), std::sin(6.5*M_PI_4)};
            static const double min_mag_k{node_size_k + node_spacing_k};
            static const double max_mag_k{min_mag_k * 2};

            const auto delta{prev - cur};
            const auto mag{delta.magnitude()};
            const double t{std::clamp((mag - min_mag_k) / max_mag_k, 0., 1.)};

            const point ne_k{lerp(ne_min_k, ne_max_k, t)};
#else
            const point ne_k{std::cos(7*M_PI_4), std::sin(7*M_PI_4)};
#endif
            const point ne_out_k{ne_k * out_scale_k};
            const point ne_stub_out_k{ne_k * out_stub_k};

            if (cur_leading) {
                // next sibling
                const point c_scale_k{1.25,1.25};
                stub_start = prev + ne_out_k;
                s = prev + ne_stub_out_k;
                c1 = prev + ne_stub_out_k * c_scale_k;
                c2 = cur + nw_in_k * c_scale_k;
                e = cur + nw_in_k;
            } else {
                // up to parent
                const point c_scale_k{1.25, 1.25};
                if (!cur_rect) {
                    stub_start = prev + ne_out_k;
                    s = prev + ne_stub_out_k;
                    c1 = prev + ne_stub_out_k * c_scale_k;
                    c2 = cur + se_in_k * c_scale_k;
                    e = cur + se_in_k;
                } else {
                    s = prev + ne_out_k;
                    c1 = prev + ne_out_k * c_scale_k;
                    //c2 = cur + se_in_k * c_scale_k;
                    e = cur;
                    e.x += node_size_k + arrowhead_offset_k.x;
                    e.y += node_size_k / 2;
                    c2 = e + point{node_size_k / 2, 0};
                }
            }
        }

        if (stub_start != point{}) {
            result.push_back(xml_node{
                "line",
                {
                    { "id", "edge_" + std::to_string(++edge_count) },
                    { "x1", std::to_string(stub_start.x) },
                    { "y1", std::to_string(stub_start.y) },
                    { "x2", std::to_string(s.x) },
                    { "y2", std::to_string(s.y) },
                    { "fill", "transparent" },
                    { "stroke", "black" },
                    { "stroke-width", "2" },
                }});
        }

        result.push_back(xml_node{
            "path",
            {
                { "id", "edge_" + std::to_string(++edge_count) },
                { "d", make_cubic_curve(s, c1, c2, e) },
                { "fill", "transparent" },
                { "stroke", "black" },
                { "stroke-width", std::to_string(stroke_width_k) },
                { "marker-end", "url(#arrowhead)" },
            }});

#if 0
        // Print the control points of the curve. Save these for debugging.
        result.push_back(xml_node{
            "line",
            {
                { "id", "edge_" + std::to_string(++edge_count) },
                { "x1", std::to_string(s.x) },
                { "y1", std::to_string(s.y) },
                { "x2", std::to_string(c1.x) },
                { "y2", std::to_string(c1.y) },
                { "fill", "transparent" },
                { "stroke", "green" },
                { "stroke-width", "2" },
            }});

        result.push_back(xml_node{
            "line",
            {
                { "id", "edge_" + std::to_string(++edge_count) },
                { "x1", std::to_string(e.x) },
                { "y1", std::to_string(e.y) },
                { "x2", std::to_string(c2.x) },
                { "y2", std::to_string(c2.y) },
                { "fill", "transparent" },
                { "stroke", "green" },
                { "stroke-width", "2" },
            }});
#endif

        prev_rect = cur_rect;
        prev_leading = cur_leading;
        prev = cur;

        ++first;
    }

    return result;
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
    auto height = derive_height(counts);
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

        const xml_node circle_k{
            "circle",
            {
                { "r", std::to_string(node_radius_k) },
                { "fill", "white" },
                { "stroke", node_properties._color },
                { "stroke-width", std::to_string(stroke_width_k) },
                { "stroke-dasharray", node_properties._stroke_dasharray },
            },
            "",
            0, 0, node_radius_k*2, node_radius_k*2
        };

        static const xml_node square_k{
            "rect",
            {
                { "width", std::to_string(node_size_k) },
                { "height", std::to_string(node_size_k) },
                { "fill", "white" },
                { "stroke", "red" },
                { "stroke-width", std::to_string(stroke_width_k) },
            },
            "",
            0, 0, node_radius_k*2, node_radius_k*2
        };

        return n == root_name_k ? square_k : circle_k;
    })};

    apply_forest(svg_nodes.begin(), svg_nodes.end(), x_offsets.begin(), [](auto& a, auto& b){
        if (a._tag == "circle") {
            a._x = b + node_radius_k;
            a._attributes["cx"] = std::to_string(a._x);
        } else if (a._tag == "rect") {
            a._x = b;
            a._attributes["x"] = std::to_string(a._x);
        } else {
            throw std::runtime_error("Uknown node shape");
        }
    });

    apply_forest(svg_nodes.begin(), svg_nodes.end(), y_offsets.begin(), [](auto& a, auto& b){
        if (a._tag == "circle") {
            a._y = b + node_radius_k;
            a._attributes["cy"] = std::to_string(a._y);
        } else if (a._tag == "rect") {
            a._y = b;
            a._attributes["y"] = std::to_string(a._y);
        } else {
            throw std::runtime_error("Uknown node shape");
        }
    });

    // Derive the edges.

    auto svg_edges{derive_edges(svg_nodes)};

    // Construct the node labels.

    auto svg_labels{transcribe_forest(state._f, [](const auto& n){
        return xml_node{
            "text",
            {
                { "font-size", std::to_string(font_size_k) },
                { "text-anchor", "middle" },
                { "dominant-baseline", "central" },
            },
            n
        };
    })};

    apply_forest(svg_labels.begin(), svg_labels.end(), x_offsets.begin(), [](auto& a, auto& b){
        a._attributes["x"] = std::to_string(b + node_radius_k);
    });

    apply_forest(svg_labels.begin(), svg_labels.end(), y_offsets.begin(), [](auto& a, auto& b){
        a._attributes["y"] = std::to_string(b + node_radius_k);
    });

    // Begin constructing the final XML.

    out << "<?xml version='1.0' encoding='utf-8'?>\n";

    adobe::forest<xml_node> xml;

    // Trailing ensures the inserted elements are a child of the svg entry.
    auto p = adobe::trailing_of(xml.insert(xml.begin(), xml_node{
        "svg",
        {
            { "version", "1.1" },
            { "xmlns", "http://www.w3.org/2000/svg" },
            { "xmlns:xlink", "http://www.w3.org/1999/xlink" },
            { "width", std::to_string(width) },
            { "height", std::to_string(height) },
        }
    }));

    xml.insert(p, xml_node{
        "style",
        {},
        R"XMLCSS(        svg {
            border: 1px solid lightgrey;
            display: block;
            margin-left: auto;
            margin-right: auto;
        })XMLCSS"
    });

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
        }
    });

    auto copy_flat{[](auto& dst, auto p, auto& src){
        auto range{adobe::preorder_range(src)};
        auto first{range.begin()};
        auto last{range.end()};

        while (first != last) {
            dst.insert(p, *first);
            ++first;
        }
    }};

    copy_flat(xml, p, svg_nodes);
    copy_flat(xml, p, svg_labels);

    for (auto& node : svg_edges) {
        xml.insert(p, std::move(node));
    }

    print_xml(std::move(xml), out);
}

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/
