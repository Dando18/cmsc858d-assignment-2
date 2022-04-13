/**
 * @file parseargs.h
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-12
 * 
 */

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
    ParseArgs(int argc, char **argv, std::vector<std::string> positionalArgs = {}) noexcept
        : positionalArgs_(positionalArgs) {
        parse(argc, argv);
    }

    bool has(std::string const& key) const noexcept {
        return kvStore_.contains(key);
    }

    template <typename T, typename StrToTFunc>
    T get(std::string const& key, StrToTFunc stringToT) const {
        if (!kvStore_.contains(key)) {
            throw std::invalid_argument("Argument " + key + " not present.");
        }
        auto const& value = kvStore_.at(key);
        return stringToT(value);
    }

    int32_t getInteger(std::string const& key) const {
        return get<int32_t, int32_t(std::string const&)>(key, [](std::string const& s) -> auto { return std::stoi(s); });
    }

    std::string getString(std::string const& key) const {
        return get<std::string, std::string(std::string const&)>(key, [](std::string const& s) -> auto { return s; });
    }

private:
    std::map<std::string, std::string> kvStore_;
    std::vector<std::string> positionalArgs_;

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