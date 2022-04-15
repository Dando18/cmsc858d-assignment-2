/**
 * @file suffixarray.h
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-13
 * 
 */
#pragma once

/* stl include */
#include <algorithm>    // adjacent_find, find_if_not
#include <array>        // array
#include <exception>    // ios_base::failure
#include <fstream>      // ifstream
#include <map>          // map
#include <ostream>      // ostream
#include <string>       // string

/* tpl includes */
#include "libsais.h"    // libsais, libsais_omp

/* local includes */
#include "serial.h"     // serialize, deserialize

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
     * @param prefixTableLength build a prefix table for all k size prefixes
     * @return SuffixArray A SuffixArray object constructed on this path
     */
    static SuffixArray fromFile(std::string const& path, int32_t prefixTableLength=0) {
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
        return SuffixArray(sequence, prefixTableLength);
    }

    /**
     * @brief Creates and returns a SuffixArray from the given string.
     * @see fromFile
     * 
     * @param values sequence of characters to construct array on
     * @param prefixTableLength build a prefix table for all k size prefixes
     * @return SuffixArray A SuffixArray object constructed on this string
     */
    static SuffixArray fromString(std::string const& values, int32_t prefixTableLength=0) {
        return SuffixArray(values, prefixTableLength);
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
        serial::serialize(prefixTableSize_, ostream);
        if (prefixTableSize_ != 0) {
            /* manually serialize std::map elements -- templates got a little hairy tyring to extend serialize */
            serial::serialize(prefixTable_.size(), ostream);
            for (auto const& [key, value] : prefixTable_) {
                auto const& [start, end] = value;
                serial::serialize(key, ostream);
                serial::serialize(start, ostream);
                serial::serialize(end, ostream);
            }
        }
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
        serial::deserialize(this->prefixTableSize_, istream);
        if (prefixTableSize_ != 0) {    /* manually deserialize std::map elements */
            prefixTable_.clear();

            size_t tmpSize;
            serial::deserialize(tmpSize, istream);
            for (size_t i = 0; i < tmpSize; i += 1) {
                std::string key;
                int32_t start, end;

                serial::deserialize(key, istream);
                serial::deserialize(start, istream);
                serial::deserialize(end, istream);

                prefixTable_.insert({key, {start, end}});
            }
        }
    }

    /**
     * @brief Stream suffix array representation to output stream.
     * 
     * @param oss where to stream elements
     * @param sa a suffix array
     * @return std::ostream& stream with suffix array data placed in it
     */
    friend std::ostream& operator<<(std::ostream& oss, SuffixArray const& sa) {
        oss << "i\tA[i]\tS[A[i],N]\n";
        int32_t counter = 0;
        for (auto const& idx : sa.suffixes_) {
            oss << counter << "\t" << idx << "\t" << sa.data_.substr(idx) << "\n";
            counter += 1; 
        }
        return oss;
    }

private:
    std::string data_;
    size_t prefixTableSize_;
    std::vector<int32_t> suffixes_;
    std::map<std::string, std::pair<int32_t, int32_t>> prefixTable_;
    std::array<int32_t, 256> histogram_;

    /**
     * @brief Construct a new Suffix Array object from a string.
     * 
     * @param data string to conduct suffix array on. 
     * @param prefixTableSize build a `k` prefix table. 0 for none.
     */
    SuffixArray(std::string const& data, size_t prefixTableSize=0)
        : data_(data+"$"), prefixTableSize_(prefixTableSize), suffixes_(data_.size()) {

        this->buildSuffixArray();

        if (prefixTableSize_ != 0) {
            this->buildPrefixTable();
        }
    }

    /**
     * @brief Builds the suffix array into internal storage.
     * @throws runtime_error if `saislib` returns a non-zero error code.
     */
    void buildSuffixArray() {
        const uint8_t *rawData = reinterpret_cast<const uint8_t*>(data_.c_str());

        #if defined(_OPENMP)
        auto result = libsais_omp(rawData, suffixes_.data(), data_.size(), 0, histogram_.data(), 0);
        #else
        auto result = libsais(rawData, suffixes_.data(), data_.size(), 0, histogram_.data());
        #endif

        if (result != 0) {
            throw std::runtime_error("SAISLIB Error -- Could not construct suffix array.");
        }
    }

    /**
     * @brief Builds an internal prefix table. Allows instant jumping to the range containing any prefix of length k.
     */
    void buildPrefixTable() {
        /* capable of jumping to the suffix array interval corresponding to any prefix of length k */

        const uint32_t k = this->prefixTableSize_;
        const size_t dataLen = data_.size();
        auto const& data = data_;   /* local reference for lambda capture */
        
        /* find if two strings have same prefix; done in-place on data_ and with early exit */
        auto hasSamePrefix = [&data, k](int32_t idx1, int32_t idx2) -> bool { return data.compare(idx1, k, data, idx2, k) == 0; };

        /* find first string with prefix length >= k */
        auto iter = std::find_if(std::begin(suffixes_), std::end(suffixes_), [k, dataLen](auto offset) { return (dataLen-offset) >= k; });

        while (iter != std::end(suffixes_)) {
            std::string prefix = data_.substr(*iter, k);

            /* find end of range */
            const auto rangeEnd = std::find_if_not(iter, std::end(suffixes_), [&hasSamePrefix, &iter](auto idx) { return hasSamePrefix(*iter, idx); });
            
            const auto start = std::distance(std::begin(suffixes_), iter);
            const auto end = std::distance(std::begin(suffixes_), rangeEnd) - 1;
            prefixTable_.insert({prefix, {start, end}});

            iter = rangeEnd;
        }
    }
};

}   // namespace suffixarray