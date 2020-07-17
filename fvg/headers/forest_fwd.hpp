/**************************************************************************************************/

#ifndef FORESTVG_FOREST_FWD_HPP
#define FORESTVG_FOREST_FWD_HPP

/**************************************************************************************************/

#if !defined(FORESTVG_USING_STLAB_FOREST)
#define FORESTVG_USING_STLAB_FOREST() 1
#endif // defined(FORESTVG_USING_STLAB_FOREST)

#if FORESTVG_USING_STLAB_FOREST()

#include <stlab/forest.hpp>

#else

#include <adobe/forest.hpp>

#endif // FORESTVG_USING_STLAB_FOREST()

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

#if FORESTVG_USING_STLAB_FOREST()

template <typename T>
using forest = stlab::forest<T>;

#define FNS stlab

#else

template <typename T>
using forest = adobe::forest<T>;

#define FNS adobe

#endif // FORESTVG_USING_STLAB_FOREST()

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/

#endif // FORESTVG_FOREST_FWD_HPP

/**************************************************************************************************/
