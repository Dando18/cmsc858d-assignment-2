/**
 * @file buildsa.cc
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-12
 * 
 */

#include <cstdlib>      // exit
#include <exception>    // invalid_argument
#include <iostream>     // cerr, cout
#include <string>       // string

#include "parseargs.h"  // ParseArgs

int main(int argc, char **argv) {
    parseargs::ParseArgs args(argc, argv, {"reference", "output"});

    int preftab;
    std::string referencePath, outputPath;
    try {
        preftab = args.getInteger("preftab");
        referencePath = args.getString("reference");
        outputPath = args.getString("output");
    } catch (std::invalid_argument const& e) {
        std::cerr << "Usage: " << argv[0] << " reference output <?--preftab>\n";
        std::exit(1);
    }

    std::cout << preftab << " " << referencePath << " " << outputPath << "\n";
}