/**************************************************************************************************/

#include <fstream>

#include "json.hpp"

/**************************************************************************************************/

namespace fvg {

/**************************************************************************************************/

json_t parse_json(const std::string& json_raw) {
    try {
        return json_t::parse(json_raw);
    } catch (const std::exception& error) {
        throw;
    }
}

/**************************************************************************************************/

json_t slurp_json(const std::filesystem::path& path) {
    std::ifstream input{path, std::ios::in | std::ios::binary};

    if (!input)
        return json_t();

    std::string json_raw;

    input.seekg(0, std::ios::end);

    json_raw.resize(input.tellg());

    input.seekg(0, std::ios::beg);

    input.read(&json_raw[0], json_raw.size());

    return json_raw.empty() ? json_t() : parse_json(json_raw);
}

/**************************************************************************************************/

std::string ddj(const json_t& j) {
    return j.dump(4);
}

/**************************************************************************************************/

std::vector<std::pair<std::string, std::string>> parameterize(const json_object& parameters) {
    std::vector<std::pair<std::string, std::string>> result;

    for (const auto& param : parameters) {
        std::string value;

        switch (param.second.type()) {
            case json_t::value_t::null: {
                value = "null";
                break;
            }
            case json_t::value_t::number_integer: {
                value = std::to_string(as<json_int>(param.second));
                break;
            }
            case json_t::value_t::number_float: {
                value = std::to_string(as<double>(param.second));
                break;
            }
            case json_t::value_t::boolean: {
                value = param.second.get<bool>() ? "true" : "false";
                break;
            }
            case json_t::value_t::string: {
                value = as<std::string>(param.second);
                break;
            }
            default: {
                throw std::runtime_error("url parameter error");
            }
        }

        if (param.first.empty() || value.empty())
            continue;

        result.push_back(std::make_pair(param.first, std::move(value)));
    }

    return result;
}

/**************************************************************************************************/

} // namespace fvg

/**************************************************************************************************/
