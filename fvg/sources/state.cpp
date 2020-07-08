/**************************************************************************************************/

// identity
#include "state.hpp"

// application
// #include "json.hpp"
// #include "state.hpp"
// #include "forest_algorithms.hpp"

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

auto make_state_nodes(const json_array& array) {
    node_map result;

    for (const auto& n : array) {
        if (!n.is_object()) {
            throw std::runtime_error("dictionary expected for node property");
        }

        const auto id{get<std::string>(n, "_id")};

        result[std::move(id)] = node_properties {
            get<std::string>(n, "color"),
            get<std::string>(n, "stroke-dasharray"),
        };
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
        make_state_nodes(get<json_array>(j, "nodes")),
        make_state_graph_settings(get<json_object>(j, "settings")),
    };
}

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/
