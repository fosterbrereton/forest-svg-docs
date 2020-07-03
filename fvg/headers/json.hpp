/**************************************************************************************************/

#ifndef FORESTVG_JSON_HPP
#define FORESTVG_JSON_HPP

/**************************************************************************************************/

#include <type_traits>
#include <filesystem>

#include <boost/logic/tribool.hpp>

#include <nlohmann/json.hpp>

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

using json_t = nlohmann::json;

/**************************************************************************************************/

using json_object = json_t::object_t;
using json_array = json_t::array_t;
using json_int = json_t::number_integer_t;
using json_uint = json_t::number_unsigned_t;
using json_float = json_t::number_float_t;

/**************************************************************************************************/

json_t parse_json(const std::string& json_raw);

json_t slurp_json(const std::filesystem::path& path);

std::string ddj(const json_t& j); // debug dump json

/**************************************************************************************************/

template <typename T>
inline bool json_value(const json_t& json, T& value);

template <>
inline bool json_value(const json_t& json, std::size_t& value) {
    bool is_number = json.is_number();

    if (is_number) {
        value = json.get<int>();
    }

    return is_number;
}

template <>
inline bool json_value(const json_t& json, bool& value) {
    bool is_bool = json.is_boolean();

    if (is_bool) {
        value = json.get<bool>();
    }

    return is_bool;
}

template <>
inline bool json_value(const json_t& json, boost::tribool& value) {
    value = json.is_boolean() ?
                boost::tribool(json.get<bool>()) :
                boost::tribool(boost::indeterminate);

    return true;
}

template <>
inline bool json_value(const json_t& json, std::string& value) {
    bool is_string = json.is_string();

    if (is_string) {
        value = json.get<std::string>();
    }

    return is_string;
}

template <typename T>
inline bool json_value(const json_t& json, T& value, T fallback) {
    bool success = json_value(json, value);

    if (!success)
        value = std::move(fallback);

    return success;
}

template <typename T>
const T& get_no_value() {
    static const T value{};
    return value;
}

template <typename T> struct jget_return_type { using type = const T&; };
template <> struct jget_return_type<boost::tribool> { using type = boost::tribool; };
template <> struct jget_return_type<json_int> { using type = json_int; };
template <> struct jget_return_type<json_uint> { using type = json_uint; };
template <> struct jget_return_type<json_float> { using type = json_float; };

template <typename T>
inline auto as(const json_t& json) -> typename jget_return_type<T>::type {
    return json.is_null() ? get_no_value<T>() : json.get_ref<const T&>();
}

template <>
inline auto as<boost::tribool>(const json_t& json) -> jget_return_type<boost::tribool>::type {
    if (json.is_boolean()) {
        return json.get_ref<const bool&>();
    }
    return get_no_value<boost::tribool>();
}

template <typename T, typename Key>
inline auto get(const json_t& json, const Key& key) -> typename jget_return_type<T>::type {
    if constexpr (std::is_integral<Key>::value) {
        if (json.size() <= key) {
            return get_no_value<T>();
        }
    } else {
        if (!json.count(key)) {
            return get_no_value<T>();
        }
    }

    return as<T>(json[key]);
}

template <typename T, typename Key, typename... Args>
inline const T& get(const json_t& json, const Key& key, Args&&... args) {
    if constexpr (std::is_integral<Key>::value) {
        if (json.size() <= key) {
            return get_no_value<T>();
        }
    } else {
        if (!json.count(key)) {
            return get_no_value<T>();
        }
    }

    return get<T>(json[key], std::forward<Args>(args)...);
}

inline const std::string& get_user_id(const json_t& json) {
    auto found = json.find("user");

    if (found == json.end()) {
        return get_no_value<std::string>();
    }

    const auto& user{*found};

    if (user.is_string()) {
        return as<std::string>(user);
    } else if (user.is_object()) {
        return get<std::string>(user, "id");
    }

    return get_no_value<std::string>();
}

/**************************************************************************************************/

std::vector<std::pair<std::string, std::string>> parameterize(const json_object& parameters);

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/

#endif // FORESTVG_JSON_HPP

/**************************************************************************************************/
