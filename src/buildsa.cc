/**
 * @file buildsa.cc
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-12
 * 
 */

/* stl includes */
#include <cstdlib>      // exit
#include <exception>    // invalid_argument
#include <iostream>     // cerr, cout
#include <string>       // string

/* local includes */
#include "parseargs.h"      // ParseArgs
#include "suffixarray.h"    // SuffixArray

int main(int argc, char **argv) {
    parseargs::ParseArgs args(argc, argv, {"reference", "output"});

    int preftab;
    std::string referencePath, outputPath;
    try {
        if (args.has("preftab")) {
            preftab = args.getInteger("preftab");
        }
        referencePath = args.getString("reference");
        outputPath = args.getString("output");
    } catch (std::invalid_argument const& e) {
        std::cerr << "Usage: " << argv[0] << " reference output <?--preftab>\n";
        std::exit(1);
    }

    auto suffixarray = suffixarray::SuffixArray::fromFile(referencePath);

    suffixarray.save(outputPath);
}