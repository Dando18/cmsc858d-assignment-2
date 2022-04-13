/**
 * @file suffixarray.h
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-13
 * 
 */
#pragma once

/* stl include */
#include <array>    // array
#include <fstream>  // ifstream
#include <string>   // string

/* tpl includes */
#include "libsais.h"

/* local includes */
#include "serial.h"

namespace suffixarray {

class SuffixArray {
    constexpr static uint32_t FILE_MAGIC = 0xabeefdad;

public:

    /**
     * @brief Creates and returns a SuffixArray from the specified path.
     * @throws std::ios_base::failure if file open/read error
     * @see fromString
     * 
     * @param path path to FASTA formatted file
     * @return SuffixArray A SuffixArray object constructed on this path
     */
    static SuffixArray fromFile(std::string const& path) {
        std::ifstream inputFile(path);
        if (!inputFile) {
            throw std::ios_base::failure("File \"" + path + "\" could not be opened.");
        }

        std::string sequence, line;
        while (std::getline(inputFile, line)) {
            if (line.starts_with(">")) {
                continue;
            }
            sequence += line;
        }
        return SuffixArray(sequence);
    }

    /**
     * @brief Creates and returns a SuffixArray from the given string.
     * @see fromFile
     * 
     * @param values sequence of characters to construct array on
     * @return SuffixArray A SuffixArray object constructed on this string
     */
    static SuffixArray fromString(std::string const& values) {
        return SuffixArray(values);
    }

    /**
     * @brief The underlying string data.
     * 
     * @return std::string const& data that the suffix array represents.
     */
    std::string const& data() const noexcept {
        return data_;
    }

    /**
     * @brief Saves the SuffixArray to the file `fname`.
     * @throws std::ios_base::failure on i/o errors
     * @see load
     * 
     * @param fname File to save data to.
     */
    void save(std::string const& fname) const {
        std::ofstream outputFile(fname);
        if (!outputFile) {
            throw std::ios_base::failure("Could not open \"" + fname + "\" for saving.");
        }

        serial::serialize(SuffixArray::FILE_MAGIC, outputFile);
        serial::serialize(*this, outputFile);

        outputFile.close();
    }

    /**
     * @brief Loads a SuffixArray from a file. Expects the format outputted by SuffixArray::save.
     * @throws std::ios_base::failure on i/o errors
     * @see save
     * 
     * @param fname File to load data from.
     */
    void load(std::string const& fname) {
        std::ifstream inputFile(fname);
        if (!inputFile) {
            throw std::ios_base::failure("Could not open \"" + fname + "\" for loading.");
        }

        uint32_t tmpMagic;
        serial::deserialize(tmpMagic, inputFile);
        if (tmpMagic != SuffixArray::FILE_MAGIC) {
            throw std::ios_base::failure("Invalid suffix array file.");
        }

        serial::deserialize(*this, inputFile);

        inputFile.close();
    }

    /**
     * @brief Serializes this object into a stream.
     * @see deserialize
     * 
     * @param ostream stream to send serialized data
     */
    void serialize(std::ofstream &ostream) const {
        serial::serialize(data_, ostream);
        serial::serialize(suffixes_, ostream);
    }
    
    /**
     * @brief Deserializes a SuffixArray from a stream. Expects the serial format used by SuffixArray::serialize.
     * @see serialize
     * 
     * @param istream stream to get serial data from
     */
    void deserialize(std::ifstream &istream) {
        serial::deserialize(data_, istream);
        serial::deserialize(suffixes_, istream);
    }

private:
    std::string data_;
    size_t prefixTableSize_;
    std::vector<int32_t> suffixes_;
    std::array<int32_t, 256> histogram_;

    /**
     * @brief Construct a new Suffix Array object from a string.
     * 
     * @param data string to conduct suffix array on. 
     * @param prefixTableSize build a `k` prefix table. 0 for none.
     */
    SuffixArray(std::string const& data, size_t prefixTableSize=0)
        : data_(data+"$"), prefixTableSize_(prefixTableSize), suffixes_(data_.size()) {

        (void)prefixTableSize_;

        const uint8_t *rawData = reinterpret_cast<const uint8_t*>(data_.c_str());
        auto result = libsais(rawData, suffixes_.data(), data_.size(), 0, histogram_.data());
        if (result != 0) {
            throw std::runtime_error("SAIS Error -- Could not generate suffix array.");
        }
    }
};

}   // namespace suffixarray