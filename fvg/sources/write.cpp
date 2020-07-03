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

constexpr auto node_width_k{100};
constexpr auto node_radius_k{node_width_k / 2};
constexpr auto node_height_k{100};
constexpr auto node_spacing_k{50};
constexpr auto margin_k{25};

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
                *pos = node_width_k;
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

    auto height{(node_height_k + node_spacing_k) * max_depth - node_spacing_k};

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
        //auto old_width{*dst_first};
        *dst_first = parent_x_offset; // + (old_width - parent_x_offset) / 2;
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
            pos = result.insert(pos, margin_k + (node_height_k + node_spacing_k) * first.depth());
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

void write_svg(const state& state, const std::filesystem::path& path) {
    std::ofstream out{path, std::ios::out | std::ios::binary};

    if (!out) {
        throw std::runtime_error("error creating output file");
    }

    auto counts = child_counts(state);
    auto widths = derive_widths(counts);
    auto height = derive_height(counts);
    auto width = derive_width(widths);
    auto x_offsets = derive_x_offsets(widths);
    auto y_offsets = derive_y_offsets(state);

    fvg::print(state._f);
    std::cout << "=-=-=-=-\n";
    fvg::print(counts);
    std::cout << "=-=-=-=-\n";
    fvg::print(widths);
    std::cout << "=-=-=-=-\n";
    fvg::print(x_offsets);
    std::cout << "=-=-=-=-\n";
    fvg::print(y_offsets);

    auto svg{transcribe_forest(state._f, [](const auto& n){
        return xml_node{
            "circle",
            {
                { "r", std::to_string(node_radius_k) },
                { "fill", "white" },
                { "stroke", "blue" },
                { "stroke-width", "5" },
            }
        };
    })};

    apply_forest(svg.begin(), svg.end(), x_offsets.begin(), [](auto& a, auto& b){
        a._attributes["cx"] = std::to_string(b + node_radius_k);
    });

    apply_forest(svg.begin(), svg.end(), y_offsets.begin(), [](auto& a, auto& b){
        a._attributes["cy"] = std::to_string(b + node_radius_k);
    });

    out << "<?xml version='1.0' encoding='utf-8'?>\n";

    adobe::forest<xml_node> xml;
    auto p = xml.insert(xml.begin(), xml_node{
        "svg",
        {
            { "version", "1.1" },
            { "xmlns", "http://www.w3.org/2000/svg" },
            { "xmlns:xlink", "http://www.w3.org/1999/xlink" },
            { "width", std::to_string(width) },
            { "height", std::to_string(height) },
        }
    });

    // so the inserted elements are a child of the svg entry.
    p = adobe::trailing_of(p);

    xml.insert(p, xml_node{
        "style",
        {},
        R"XMLCSS(svg {
    border: 1px solid lightgrey;
    display: block;
    margin-left: auto;
    margin-right: auto;
})XMLCSS"
    });

    auto range{adobe::preorder_range(svg)};
    auto first{range.begin()};
    auto last{range.end()};

    while (first != last) {
        xml.insert(p, *first);
        ++first;
    }

    print_xml(std::move(xml), out);
}

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/
