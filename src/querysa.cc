/**
 * @file querysa.cc
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-12
 * 
 */

#include <iostream>
#include <ranges>

#include "parseargs.h"      // ParseArgs
#include "suffixarray.h"    // SuffixArray
#include "utilities.h"      // parseFastaQueries

/* forward declarations */
std::vector<suffixarray::SuffixArray::Query> parseFastaQueries(std::string const& fname);

int main(int argc, char **argv) {
    using namespace suffixarray;

    parseargs::ParseArgs args(argc, argv, {"index", "queries", "query mode", "output"});

    std::string indexPath, queriesPath, queryModeStr, outputPath;
    try {
        indexPath = args.getString("index");
        queriesPath = args.getString("queries");
        queryModeStr = args.getString("query mode");
        outputPath = args.getString("output");
    } catch (std::invalid_argument const& e) {
        std::cerr << "Usage: " << argv[0] << " index queries query-mode output\n";
        std::exit(1);
    }

    auto suffixArray = SuffixArray::fromSave(indexPath);
    SuffixArray::QueryMode queryMode = (queryModeStr == "naive") ? SuffixArray::Naive : SuffixArray::SimpleAccelerant;
    utilities::Timer timer;

    /* perform queries */
    auto queries = parseFastaQueries(queriesPath);
    timer.start();
    suffixArray.queries(std::begin(queries), std::end(queries), queryMode);
    timer.stop();
    const auto duration = timer.millisecondsElapsed();
    const auto avgDuration = duration / queries.size();
    std::cout << suffixArray.data().size() << "," << suffixArray.getPrefixTableSize() << ","
        << queryModeStr << "," << queries.size() << "," << duration << "," << avgDuration << "\n";
    
    if (outputPath != "+") {
        std::ofstream outputFile(outputPath);
        for (auto const& q : queries) {
            outputFile << q.title << '\t' << q.result.size();
            for (auto const& index : q.result) {
                outputFile << '\t' << index;
            }
            outputFile << '\n';
        }
        outputFile.flush();
        outputFile.close();
    }
}


std::vector<suffixarray::SuffixArray::Query> parseFastaQueries(std::string const& fname) {
    std::ifstream inputFile(fname);
    if (!inputFile) {
        throw std::ios_base::failure("Could not open " + fname + " for reading.");
    }
    
    std::vector<suffixarray::SuffixArray::Query> queries;

    std::string line, currentTitle, currentQuery;
    while (std::getline(inputFile, line)) {
        if (line.starts_with('>')) {
            // flush previous query
            if (currentQuery != "") {
                queries.push_back({currentTitle, currentQuery, {}});
                currentQuery = "";
            }

            // read title
            currentTitle = line.substr(1);
        } else {
            // append to current fasta query
            currentQuery += line;
        }
    }
    queries.push_back({currentTitle, currentQuery, {}});
    
    return queries;
}