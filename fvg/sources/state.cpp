/**************************************************************************************************/

// identity
#include "state.hpp"

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

namespace detail {

/**************************************************************************************************/

void make_state_forest(node_forest& f, node_iterator p, const json_array& array) {
    node_iterator last_node{f.end()};
    for (const auto& n : array) {
        if (n.is_string()) {
            last_node = f.insert(p, as<std::string>(n));
        } else if (n.is_array()) {
            if (last_node == f.end()) {
                throw std::runtime_error("children missing parent definition");
            }

            make_state_forest(f, adobe::trailing_of(last_node), as<json_array>(n));

            last_node = f.end();
        } else {
            throw std::runtime_error("unexpected node type; must be string or array");
        }
    }
}

/**************************************************************************************************/

} // namespace detail

/**************************************************************************************************/

auto make_state_forest(const json_array& array) {
    node_forest result;

    detail::make_state_forest(result, result.begin(), array);

    return result;
}

/**************************************************************************************************/

template <typename T>
bool maybe_get(T& f, const json_t& j, const std::string& k) {
    if (!j.count(k)) return false;

    f = get<T>(j, k);

    return true;
}

/**************************************************************************************************/

template <>
bool maybe_get(extents& f, const json_t& j, const std::string& k) {
    double value{0};

    if (maybe_get(value, j, k)) {
        f.l = value;
        f.t = value;
        f.r = value;
        f.b = value;
    }

    if (maybe_get(value, j, k + "_width")) {
        f.l = value;
        f.r = value;
    }

    if (maybe_get(value, j, k + "_height")) {
        f.t = value;
        f.b = value;
    }

    maybe_get(f.l, j, k + "_left");
    maybe_get(f.t, j, k + "_top");
    maybe_get(f.r, j, k + "_right");
    maybe_get(f.b, j, k + "_bottom");

    return true;
}

/**************************************************************************************************/

auto make_state_nodes(const json_object& object) {
    node_map result;

    for (const auto& entry : object) {
        const auto& mapped{entry.second};

        if (!mapped.is_object()) {
            throw std::runtime_error("dictionary expected for node property");
        }

        node_properties value;

        maybe_get(value._color, mapped, "color");
        maybe_get(value._stroke_dasharray, mapped, "stroke-dasharray");

        result[entry.first] = std::move(value);
    }

    return result;
}

/**************************************************************************************************/

auto make_state_edges(const json_object& object) {
    edge_map result;

    for (const auto& entry : object) {
        const auto& mapped{entry.second};

        if (!mapped.is_object()) {
            throw std::runtime_error("dictionary expected for node property");
        }

        edge_properties value;

        maybe_get(value._hide, mapped, "hide");
        maybe_get(value._color, mapped, "color");
        maybe_get(value._t, mapped, "t");
        maybe_get(value._stroke_dasharray, mapped, "stroke-dasharray");
        maybe_get(value._label_offset, mapped, "label_offset");
        maybe_get(value._text_anchor, mapped, "text-anchor");

        result[entry.first] = std::move(value);
    }

    return result;
}

/**************************************************************************************************/

auto make_state_edge_labels(const json_array& array) {
    edge_labels result;

    for (const auto& l : array) {
        if (!l.is_string()) {
            throw std::runtime_error("string expected for edge label");
        }

        result.push_back(as<std::string>(l));
    }

    return result;
}

/**************************************************************************************************/

auto make_state_graph_settings(const json_object& object) {
    graph_settings result;

    maybe_get(result._with_root, object, "with_root");
    maybe_get(result._with_leaf_edges, object, "with_leaf_edges");
    maybe_get(result._margin, object, "margin");

    return result;
}

/**************************************************************************************************/

state make_state(const json_t& j) {
    return {
        make_state_forest(get<json_array>(j, "forest")),
        make_state_nodes(get<json_object>(j, "nodes")),
        make_state_edges(get<json_object>(j, "edges")),
        make_state_edge_labels(get<json_array>(j, "edge_labels")),
        make_state_graph_settings(get<json_object>(j, "settings")),
    };
}

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/
