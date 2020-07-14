/**************************************************************************************************/

#ifndef FORESTVG_STATE_HPP
#define FORESTVG_STATE_HPP

/**************************************************************************************************/

// stdc++
#include <unordered_map>

// asl
#include <adobe/forest.hpp>

// application
#include "json.hpp"

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

struct node_properties {
    std::string _color{"blue"};
    std::string _stroke_dasharray{"0"};
    std::string _leading_label;
    std::string _trailing_label;
};

struct edge_properties {
    bool _hide{false};
    std::string _color{"black"};
    double _t{0.5}; // Range (0..1). Bezier point where label will go. Unused as of now.
};

struct graph_settings {
    bool _with_root{false};
    bool _with_leaf_edges{true};
};

using node_forest = adobe::forest<std::string>;
using node_iterator = node_forest::iterator;
using node_map = std::unordered_map<std::string, node_properties>;
using edge_map = std::unordered_map<std::string, edge_properties>;
using edge_labels = std::vector<std::string>;

struct state {
    adobe::forest<std::string> _f;
    node_map _n;
    edge_map _e;
    edge_labels _l;
    graph_settings _s;
};

/**************************************************************************************************/

state make_state(const fvg::json_t& j);

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/

#endif // FORESTVG_STATE_HPP

/**************************************************************************************************/
