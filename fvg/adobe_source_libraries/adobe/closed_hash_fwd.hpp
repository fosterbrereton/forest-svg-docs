/*
    Copyright 2013 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
/*************************************************************************************************/

#ifndef ADOBE_CLOSED_HASH_FWD_HPP
#define ADOBE_CLOSED_HASH_FWD_HPP

/*************************************************************************************************/

#include <adobe/config.hpp>

#include <functional>
#include <utility>

#include <adobe/functional.hpp>
#include <adobe/memory_fwd.hpp>
#include <adobe/utility.hpp>

/*************************************************************************************************/

namespace adobe {
namespace version_1 {

/*************************************************************************************************/

template <typename T, typename KeyTransform = identity<const T>, typename Hash = std::hash<T>,
          typename Pred = std::equal_to<T>, typename A = capture_allocator<T>>
class closed_hash_set;

template <typename Key, typename T, typename Hash = std::hash<Key>,
          typename Pred = std::equal_to<Key>, typename A = capture_allocator<std::pair<Key, T>>>
class closed_hash_map;

/*************************************************************************************************/

} // namespace version_1

using version_1::closed_hash_set;
using version_1::closed_hash_map;

#if defined(ADOBE_NO_DOCUMENTATION)
/* REVISIT (mmarcus) : doxygen doesn't seem to understand using
declarartions.  This is a doxygen only hack to keep reference links
from breaking.
*/
//!\ingroup abi_container
typedef version_1::closed_hash_set closed_hash_set;
typedef version_1::closed_hash_map closed_hash_map;
#endif

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
