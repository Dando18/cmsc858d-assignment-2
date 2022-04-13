/**
 * @file serial.h
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-13
 */
#pragma once
#include <cstdint>
#include <type_traits>

namespace serial {

/* forward declarations */
class ofstream;
class ifstream;

/**
 * @brief Concept resolves if T is a container as described in the standard "Container Named Requirements".
 * Concept definition is modified from https://stackoverflow.com/a/60450396/3769237
 */
template <typename T>
concept Container = requires(T a, const T b) {
    requires std::regular<T>;
    requires std::swappable<T>;
    requires std::destructible<typename T::value_type>;
    requires std::same_as<typename T::reference, typename T::value_type &>;
    requires std::same_as<typename T::const_reference, const typename T::value_type &>;
    requires std::forward_iterator<typename T::iterator>;
    requires std::forward_iterator<typename T::const_iterator>;
    requires std::signed_integral<typename T::difference_type>;
    requires std::same_as<typename T::difference_type, typename std::iterator_traits<typename T::iterator>::difference_type>;
    requires std::same_as<typename T::difference_type, typename std::iterator_traits<typename T::const_iterator>::difference_type>;
    { a.begin() } -> std::same_as<typename T::iterator>;
    { a.end() } -> std::same_as<typename T::iterator>;
    { b.begin() } -> std::same_as<typename T::const_iterator>;
    { b.end() } -> std::same_as<typename T::const_iterator>;
    { a.cbegin() } -> std::same_as<typename T::const_iterator>;
    { a.cend() } -> std::same_as<typename T::const_iterator>;
    { a.size() } -> std::same_as<typename T::size_type>;
    { a.max_size() } -> std::same_as<typename T::size_type>;
    { a.empty() } -> std::same_as<bool>;
};

/**
 * @brief Concept resolves if T implements a resize function. 
 */
template <typename T>
concept Resizable = requires(T a, size_t size) {
    { a.resize(size) } -> std::same_as<void>;
};

/**
 * @brief Concept resolves if T implements a serialize and deserialize function.
 */
template<typename T>
concept SerializeOverloads = requires(const T a, T b, std::ofstream& out, std::ifstream& in) {
    { a.serialize(out) } -> std::same_as<void>; 
    { b.deserialize(in) } -> std::same_as<void>;
};

/**
 * @brief Concept resolves if T is trivial data type, container, or provides serialization functions.
 * @note "Serializable" is a bit of a misnomer. If T is a container, then it is not necessarily serializable. It's
 *       sub-data type may not be serializable, but concepts don't allow recursive definitions. This is still useful 
 *       for the serialize/deserialize functions though.
 * @see std::is_trivial
 * @see Container 
 */
template <typename T>
concept Serializable = std::is_trivial<T>::value || Container<T> || SerializeOverloads<T>;

/**
 * @brief Serialize an objects bytes into an ofstream. If trivial (POD, bare struct with simple extant, etc...), 
 *        then this will just write out the data. If DataType is a container, then serialize will be recursively called
 *        on each value. If DataType::serialize exists, then this will be used.
 * @see deserialize
 * 
 * @tparam DataType serializable
 * @param data data to serialize
 * @param outputStream location of resulting data
 */
template <Serializable DataType>
void serialize(DataType const& data, std::ofstream &outputStream) {

    if constexpr (SerializeOverloads<DataType>) {
        data.serialize(outputStream);
    } else if constexpr (Container<DataType>) {
        const auto size = data.size();
        outputStream.write(reinterpret_cast<char const*>(&size), sizeof(size));
        for (auto const& value : data) {    
            serialize(value, outputStream);
        }
    } else {
        outputStream.write(reinterpret_cast<char const*>(&data), sizeof(data));
    }
}


/**
 * @brief Deserialize bytes from an ifstream into data object. If trivial (POD, bare struct with simple extant, etc...), 
 *        then this will just read in the data. If DataType is a container, then deserialize will be recursively called
 *        on each value. If DataType::deserialize exists, then it will be used. Expects the format/ordering used 
 *        by `serialize` for containers.
 * @see serialize
 * 
 * @throws std::runtime_error Thrown when deserializing a container, there's a size mismatch between current container
 *         and one in data, and the DataType::resize(size_t) does not exist.
 * 
 * @tparam DataType serializable
 * @param data where to write data
 * @param inputStream location of incoming data
 */
template <Serializable DataType>
void deserialize(DataType &data, std::ifstream &inputStream) {

    if constexpr (SerializeOverloads<DataType>) {
        data.deserialize(inputStream);
    } else if constexpr (Container<DataType>) {
        const auto currentSize = data.size();
        auto size = decltype(currentSize){};

        inputStream.read(reinterpret_cast<char*>(&size), sizeof(size));
        if (currentSize != size) {
            if constexpr (Resizable<DataType>) {
                data.resize(size);
            } else {
                throw std::runtime_error("Container size mismatch during deserialization. Cannot recover.");
            }
        }

        for (auto &value : data) {
            deserialize(value, inputStream);
        }
    } else {
        inputStream.read(reinterpret_cast<char *>(&data), sizeof(data));
    }
}

}   // end namespace serialize