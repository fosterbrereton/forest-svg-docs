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

auto make_state_nodes(const json_object& object) {
    node_map result;

    for (const auto& entry : object) {
        const auto& mapped{entry.second};

        if (!mapped.is_object()) {
            throw std::runtime_error("dictionary expected for node property");
        }

        result[entry.first] = node_properties {
            get<std::string>(mapped, "color"),
            get<std::string>(mapped, "stroke-dasharray"),
        };
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

        if (mapped.count("_hide")) value._hide = get<bool>(mapped, "_hide");
        if (mapped.count("_color")) value._color = get<std::string>(mapped, "_color");
        if (mapped.count("t")) value._t = get<double>(mapped, "t");

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
    graph_settings result{
        get<bool>(object, "with_root"),
        !get<bool>(object, "no_leaf_edges"),
    };

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
