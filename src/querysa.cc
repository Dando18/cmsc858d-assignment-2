/**
 * @file querysa.cc
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-12
 * 
 */

#include <iostream>

#include "parseargs.h"

int main(int argc, char **argv) {
    parseargs::ParseArgs args(argc, argv, {"index", "queries", "query mode", "output"});

    std::string indexPath, queriesPath, queryMode, outputPath;
    try {
        indexPath = args.getString("index");
        queriesPath = args.getString("queries");
        queryMode = args.getString("query mode");
        outputPath = args.getString("output");
    } catch (std::invalid_argument const& e) {
        std::cerr << "Usage: " << argv[0] << " index queries query-mode output\n";
        std::exit(1);
    }

    std::cout << indexPath << " " << queriesPath << " " << queryMode << " " << outputPath << "\n";
}