/**************************************************************************************************/

#ifndef FORESTVG_WRITE_HPP
#define FORESTVG_WRITE_HPP

/**************************************************************************************************/

// stdc++
#include <filesystem>

// application
#include "state.hpp"

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

void write_svg(state state, const std::filesystem::path& path);

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/

#endif // FORESTVG_WRITE_HPP

/**************************************************************************************************/
