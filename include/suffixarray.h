/**
 * @file suffixarray.h
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-13
 * 
 */
#pragma once

/* stl include */
#include <algorithm>    // find, find_if_not
#include <array>        // array
#include <exception>    // ios_base::failure
#include <execution>    // execution::par_unseq
#include <fstream>      // ifstream
#include <ostream>      // ostream
#include <random>       // 
#include <string>       // string
#include <unordered_map>// unordered_map

/* tpl includes */
#include "libsais.h"    // libsais, libsais_omp

/* local includes */
#include "serial.h"     // serialize, deserialize
#include "utilities.h"  // Timer


namespace suffixarray {

class SuffixArray {
    constexpr static uint32_t FILE_MAGIC = 0xabeefdad;
    typedef std::unordered_map<std::string, std::pair<int32_t, int32_t>> PrefixTable;

public:
    enum QueryMode { Naive, SimpleAccelerant };
    struct Query { std::string title, query; std::vector<int32_t> result; };
    

    /**
     * @brief Creates and returns a SuffixArray from the specified path.
     * @throws std::ios_base::failure if file open/read error
     * @see fromString
     * 
     * @param path path to FASTA formatted file
     * @param prefixTableLength build a prefix table for all k size prefixes
     * @return SuffixArray A SuffixArray object constructed on this path
     */
    static SuffixArray fromFASTAFile(std::string const& path, int32_t prefixTableLength=0) {
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
     * @brief Load a SuffixArray from a previously save SuffixArray
     * 
     * @param path file path to saved SuffixArray
     * @return SuffixArray the suffix array from the specified file
     */
    static SuffixArray fromSave(std::string const& path) {
        SuffixArray sa("");
        sa.load(path);
        return sa;
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
     * @brief Query the SuffixArray for the specific Query. Query object specifies the query string
     * and will hold the result of the query.
     * If a prefix table exists, then it will be used to accelerate the query.
     * 
     * @param q Query object. Also receives the query results.
     * @param mode Whether to use Naive or SimpleAccelerant method.
     */
    void query(Query &q, QueryMode mode = QueryMode::Naive) const {
        auto searchStart = std::begin(suffixes_);
        auto searchEnd = std::end(suffixes_);

        /* narrow range with prefix table if possible */
        if (!prefixTable_.empty() && q.query.size() >= prefixTableSize_) {
            const auto subQuery = q.query.substr(prefixTableSize_);
            if (prefixTable_.contains(subQuery)) {
                searchStart = std::next(std::begin(suffixes_), prefixTable_.at(subQuery).first);
                searchEnd = std::next(std::begin(suffixes_), prefixTable_.at(subQuery).second);
            }
        }

        /*  unfortunately std::equal_ranges won't work here because the search object has to 
            have the same type as the container; i.e. you can't search on a well-ordered predicate, but have to 
            search for a value; boost::transform_iterator's would make this a lot cleaner */

        /* lower_bound */
        auto [lower, upper] = std::tuple(searchStart, searchEnd);
        auto range = std::distance(lower, upper);
        while (range > 0) {
            auto iter = lower;
            auto step = range / 2;
            std::advance(iter, step);

            size_t searchOffset = 0;
            if (mode == SimpleAccelerant) {
                const auto lowerLCP = utilities::LCPLength(std::cbegin(q.query), std::cend(q.query), std::next(std::begin(data_), *lower));
                const auto upperLCP = utilities::LCPLength(std::cbegin(q.query), std::cend(q.query), std::next(std::begin(data_), *iter));
                searchOffset = std::min(lowerLCP, upperLCP);
            }
            const auto searchLength = q.query.size() - searchOffset;            

            if (data_.compare((*iter)+searchOffset, searchLength, q.query, searchOffset, searchLength) < 0) {  /* true if data_[] < queryStr */
                lower = ++iter;
                range -= step + 1;
            } else {
                range = step;
            }
        }
        const auto lowerIndex = lower;

        /* upper_bound */
        std::tie(lower, upper) = std::tuple(searchStart, searchEnd);
        range = std::distance(lower, upper);
        while (range > 0) {
            auto iter = lower;
            auto step = range / 2;
            std::advance(iter, step);

            size_t searchOffset = 0;
            if (mode == SimpleAccelerant) {
                const auto lowerLCP = utilities::LCPLength(std::cbegin(q.query), std::cend(q.query), std::next(std::begin(data_), *lower));
                const auto upperLCP = utilities::LCPLength(std::cbegin(q.query), std::cend(q.query), std::next(std::begin(data_), *iter));
                searchOffset = std::min(lowerLCP, upperLCP);
            }
            const auto searchLength = q.query.size() - searchOffset; 

            if (q.query.compare(searchOffset, searchLength, data_, (*iter)+searchOffset, searchLength) >= 0) {
                lower = ++iter;
                range -= step + 1;
            } else {
                range = step;
            }
        }
        const auto upperIndex = lower;

        q.result = std::vector<int32_t>(lowerIndex, upperIndex);
    }

    /**
     * @brief Perform a set of queries. If enabled, will do them in parallel.
     * @see query
     * 
     * @tparam Iterator iterator which returns SuffixArray::Query objects.
     * @param begin Start of query range.
     * @param end End of query range.
     * @param mode Querying method.
     */
    template <typename Iterator>
    void queries(Iterator begin, Iterator end, QueryMode mode = QueryMode::Naive) {
        std::for_each(utilities::executionPolicy, begin, end, [this, mode](auto &q) { this->query(q, mode); });
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
     * @brief Returns the underlying suffix array.
     * 
     * @return std::vector<int32_t> const& Array of indices into string for each suffix.
     */
    std::vector<int32_t> const& suffixes() const noexcept {
        return suffixes_;
    }

    size_t getPrefixTableSize() const noexcept {
        return prefixTableSize_;
    }

    /**
     * @brief Returns the duration it took to build SuffixArray.
     * 
     * @return double Duration to build SuffixArray
     */
    double getSuffixArrayBuildTime() const noexcept {
        return suffixArrayBuildTime_;
    }

    /**
     * @brief Returns the duration it took to build PrefixTable. Returns 0 if no 
     * prefix table was built.
     * 
     * @return double PrefixTable build duration
     */
    double getPrefixTableBuildTime() const noexcept {
        return prefixTableBuildTime_;
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
            /* manually serialize std::map elements -- templates got a little hairy trying to extend serialize */
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
    double suffixArrayBuildTime_, prefixTableBuildTime_;

    std::vector<int32_t> suffixes_;
    PrefixTable prefixTable_;
    std::array<int32_t, 256> histogram_;

    /**
     * @brief Construct a new Suffix Array object from a string.
     * 
     * @param data string to conduct suffix array on. 
     * @param prefixTableSize build a `k` prefix table. 0 for none.
     */
    SuffixArray(std::string const& data, size_t prefixTableSize=0)
        : data_(data), prefixTableSize_(prefixTableSize), suffixes_(data_.size()+1) {

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<uint8_t> dist(0,3);
        const char ALPHABET[4] = {'A', 'T', 'G', 'C'};
        std::for_each(utilities::executionPolicy, std::begin(data_), std::end(data_), [&dist,&rng,&ALPHABET](auto &c) {
            c = std::toupper(c);
            if (c != 'A' && c != 'T' && c != 'G' && c != 'C') {
                c = ALPHABET[dist(rng)];
            }
        });
        data_.push_back('$');

        utilities::Timer timer;
        timer.start();
        this->buildSuffixArray();
        timer.stop();
        suffixArrayBuildTime_ = timer.millisecondsElapsed();

        prefixTableBuildTime_ = 0.0;
        if (prefixTableSize_ != 0) {
            timer.start();

            #if defined(_OPENMP)
            buildPrefixTableParallel();
            #else
            buildPrefixTable(std::begin(suffixes_), std::end(suffixes_), prefixTable_);
            #endif
            
            timer.stop();
            prefixTableBuildTime_ = timer.millisecondsElapsed();
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
     * 
     * @param rangeStart Beginning of range to build from.
     * @param rangeEnd end of range to build from.
     * @param prefixTable what table to build into.
     */
    void buildPrefixTable(std::vector<int32_t>::iterator rangeStart, std::vector<int32_t>::iterator rangeEnd, 
        PrefixTable &prefixTable) {
        const uint32_t k = this->prefixTableSize_;
        const size_t dataLen = data_.size();
        auto const& data = data_;   /* local reference for lambda capture */
        
        /* find if two strings have same prefix; done in-place on data_ and with early exit */
        auto hasSamePrefix = [&data, k](int32_t idx1, int32_t idx2) -> bool { 
                return data.compare(idx1, k, data, idx2, k) == 0; };

        /* find first string with prefix length >= k */
        auto iter = std::find_if(rangeStart, rangeEnd, [k, dataLen](auto offset) { return (dataLen-offset) >= k; });

        while (std::distance(iter, rangeEnd) > 0) { // iter < rangeEnd
            std::string prefix = data_.substr(*iter, k);

            const auto endOfRange = std::find_if_not(iter, std::end(suffixes_), [&hasSamePrefix, &iter](auto idx) { 
                    return hasSamePrefix(*iter, idx); });

            const auto start = std::distance(std::begin(suffixes_), iter);
            const auto end = std::distance(std::begin(suffixes_), endOfRange) - 1;
            prefixTable.insert({prefix, {start, end}});

            iter = endOfRange;
        }
    }

    /**
     * @brief Builds prefix table using multiple threads.
     * @note This could be vastly improved by a thread-safe hash table implementation.
     * 
     */
    void buildPrefixTableParallel() {
        constexpr uint32_t NUM_CHUNKS = 128;
        std::array<PrefixTable, NUM_CHUNKS> tables;

        const uint32_t k = this->prefixTableSize_;
        auto const& data = data_;   /* local reference for lambda capture */
        auto hasSamePrefix = [&data, k](int32_t idx1, int32_t idx2) -> bool { 
                return data.compare(idx1, k, data, idx2, k) == 0; };

        prefixTable_.reserve(suffixes_.size());
        for (auto &tab : tables) {
            tab.reserve((suffixes_.size()-k+1) / NUM_CHUNKS);
        }

        #pragma omp parallel for
        for (uint32_t i = 0; i < NUM_CHUNKS; i += 1) {

            /* compute starting range */
            auto iter = std::next(std::begin(suffixes_), i * suffixes_.size() / NUM_CHUNKS);
            auto endOfRange = std::next(std::begin(suffixes_), (i+1) * suffixes_.size() / NUM_CHUNKS);

            /* check if this is in an existing region -- if so seek to next region */
            if (i != 0 && hasSamePrefix(*iter, *std::prev(iter))) {
                iter = std::find_if_not(iter, endOfRange, [&hasSamePrefix, &iter](auto idx) { 
                    return hasSamePrefix(*iter, idx); });
            }

            /* build out table */
            this->buildPrefixTable(iter, endOfRange, tables.at(i));
        }

        /* rejoin tables */
        for (auto const& tab : tables) {
            prefixTable_.insert(std::begin(tab), std::end(tab));
        }
    }

};

}   // namespace suffixarray