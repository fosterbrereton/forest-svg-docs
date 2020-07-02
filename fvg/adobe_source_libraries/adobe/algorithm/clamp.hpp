/*
    Copyright 2013 Adobe
    Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*/
/**************************************************************************************************/

#ifndef ADOBE_ALGORITHM_CLAMP_HPP
#define ADOBE_ALGORITHM_CLAMP_HPP

#include <adobe/algorithm/select.hpp>
#include <adobe/functional.hpp>

#include <functional>

namespace adobe {

/**************************************************************************************************/
/*!
\defgroup clamp clamp
\ingroup sorting

As with adobe::min and adobe::max, clamp is a select algorithm. The clamp algorithm returns min if
\c x is less than \c min and \c max if \c x is greater than max.

The clamp algorithm is stable with respect to the order <code>min, x, max</code>.

The clamp_unorderd algorithm does not presume an ordering between \c min and \c max. It is
equivalent to <code>median(min, x, max)</code>.

\see
    - \ref minmax
    - \ref median
    - \ref select
*/

/**************************************************************************************************/

/*!
    \ingroup clamp
    \brief clamp implementation
    \pre <code>!r(max, min)</code>
*/

template <typename T, typename R>
inline const T& clamp(const T& x, const T& min, const T& max, R r) {
    return select_1_3_ac(min, x, max, std::bind(r, std::placeholders::_1, std::placeholders::_2));
}

/*!
    \ingroup clamp
    \brief clamp implementation
    \pre <code>!r(max, min)</code>
*/

template <typename T, typename R>
inline T& clamp(T& x, T& min, T& max, R r) {
    return select_1_3_ac(min, x, max, std::bind(r, std::placeholders::_1, std::placeholders::_2));
}

/*!
    \ingroup clamp
    \brief clamp implementation
    \pre <code>!(max < min)</code>
*/

template <typename T>
inline const T& clamp(const T& x, const T& min, const T& max) {
    return select_1_3_ac(min, x, max, less());
}

/*!
    \ingroup clamp
    \brief clamp implementation
    \pre <code>!(max < min)</code>
*/

template <typename T>
inline T& clamp(T& x, T& min, T& max) {
    return select_1_3_ac(min, x, max, less());
}

/*!
    \ingroup clamp
    \brief clamp_unordered implementation
*/

template <typename T, typename R>
inline const T& clamp_unordered(const T& x, const T& min, const T& max, R r) {
    return select_1_3(min, x, max, std::bind(r, std::placeholders::_1, std::placeholders::_2));
}

/*!
    \ingroup clamp
    \brief clamp_unordered implementation
*/

template <typename T, typename R>
inline T& clamp_unordered(T& x, T& min, T& max, R r) {
    return select_1_3(min, x, max, std::bind(r, std::placeholders::_1, std::placeholders::_2));
}

/*!
    \ingroup clamp
    \brief clamp_unordered implementation
*/

template <typename T>
inline const T& clamp_unordered(const T& x, const T& min, const T& max) {
    return select_1_3(min, x, max, less());
}

/*!
    \ingroup clamp
    \brief clamp_unordered implementation
*/

template <typename T>
inline T& clamp_unordered(T& x, T& min, T& max) {
    return select_1_3(min, x, max, less());
}

/**************************************************************************************************/

} // namespace adobe

#endif
