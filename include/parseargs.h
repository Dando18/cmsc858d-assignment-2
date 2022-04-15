/**
 * @file parseargs.h
 * @author Daniel Nichols
 * @brief Utilities for simple argument parsing.
 * @date 2022-04-12
 * 
 */

#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace parseargs {

/**
 * @brief Fairly rudimentary argument parser. One name per argument and zero or
 * one value.
 * 
 */
class ParseArgs {
public:
    /**
     * @brief Construct a new ParseArgs object. Does parsing on construction.
     * 
     * @param argc number of arguments (number of elements in argv)
     * @param argv array of char arrays representing program arguments
     * @param positionalArgs Names of arguments passed positionally and in the expected order.
     */
    ParseArgs(int argc, char **argv, std::vector<std::string> positionalArgs = {}) noexcept
        : positionalArgs_(positionalArgs) {
        parse(argc, argv);
    }

    /**
     * @brief Test if an argument was provided.
     * 
     * @param key Argument name.
     * @return true If argument was provided.
     * @return false If argument was not provided.
     */
    bool has(std::string const& key) const noexcept {
        return kvStore_.contains(key);
    }

    /**
     * @brief Retrieve a value from the parsed arguments list.
     * @throws std::invalid_argument if key is not in list.
     * @see has
     * 
     * @tparam T The type of the argument to return
     * @param key Argument name.
     * @param stringToT Function that casts a string to the expected return type T.
     * @return T return value
     */
    template <typename T>
    T get(std::string const& key, std::function<T(std::string const&)> stringToT) const {
        if (!kvStore_.contains(key)) {
            throw std::invalid_argument("Argument " + key + " not present.");
        }
        auto const& value = kvStore_.at(key);
        return stringToT(value);
    }

    /**
     * @brief Retrieve a value from the parsed arguments list with default.
     * 
     * @tparam T The type of the argument to return
     * @param key Argument name.
     * @param def Default value.
     * @param stringToT Function that casts a string to the expected return type T.
     * @return T return value
     */
    template <typename T>
    T get(std::string const& key, T const& def, std::function<T(std::string const&)> stringToT) const {
        if (!kvStore_.contains(key)) {
            return def;
        }
        auto const& value = kvStore_.at(key);
        return stringToT(value);
    }

    /**
     * @brief Wrapper for `get` for integer types.
     * 
     * @param key Argument name.
     * @return int32_t value passed to argument.
     */
    int32_t getInteger(std::string const& key) const {
        return get<int32_t>(key, [](std::string const& s) -> auto { return std::stoi(s); });
    }

    /**
     * @brief Wrapper for `get` for integer types with default value.
     * 
     * @param key Argument name.
     * @param def Default value.
     * @return int32_t value passed to argument.
     */
    int32_t getInteger(std::string const& key, int32_t def) const {
        return get<int32_t>(key, def, [](std::string const& s) -> auto { return std::stoi(s); });
    }

    /**
     * @brief Wrapper for `get` for string types.
     * 
     * @param key Argument name.
     * @return std::string value passed to argument.
     */
    std::string getString(std::string const& key) const {
        return get<std::string>(key, [](std::string const& s) -> auto { return s; });
    }

    /**
     * @brief Wrapper for `get` for string types with default.
     * 
     * @param key Argument name.
     * @param def Default value.
     * @return std::string value passed to argument.
     */
    std::string getString(std::string const& key, std::string const& def) const {
        return get<std::string>(key, def, [](std::string const& s) -> auto { return s; });
    }

private:
    std::map<std::string, std::string> kvStore_;
    std::vector<std::string> positionalArgs_;

    /**
     * @brief Parses the input arguments into a key-value store. Uses positional arguments provided to 
     * constructor to key positional arguments.
     * 
     * @param argc number of arguments (length of argv)
     * @param argv array of char arrays containing argument list
     */
    void parse(int argc, char **argv) {

        std::string lastKey = "";
        uint32_t curPositionalIndex = 0;
        for (int i = 1; i < argc; i += 1) {
            std::string_view strVal(argv[i]);
            
            if (strVal.starts_with('-')) {
                /* key name */
                strVal.remove_prefix((strVal.starts_with("--")) ? 2 : 1);
                kvStore_.insert({std::string(strVal), std::string()});
                lastKey = strVal;
            } else if (lastKey == "") {
                /* positional argument */
                kvStore_.insert({positionalArgs_.at(curPositionalIndex), std::string(strVal)});
                curPositionalIndex += 1;
            } else {
                /* keyword argument */
                kvStore_[lastKey] = std::string(strVal);
                lastKey = "";
            }
        }
    }
};

}   // namespace parseargs