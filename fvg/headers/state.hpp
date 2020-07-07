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
};

struct graph_settings {
    bool _with_root{false};
};

using node_forest = adobe::forest<std::string>;
using node_iterator = node_forest::iterator;
using node_map = std::unordered_map<std::string, node_properties>;

struct state {
    adobe::forest<std::string> _f;
    node_map _n;
    graph_settings _s;
};

/**************************************************************************************************/

state make_state(const fvg::json_t& j);

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/

#endif // FORESTVG_STATE_HPP

/**************************************************************************************************/
