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
#include <filesystem>   // file_size
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
        preftab = args.getInteger("preftab", 0);
        referencePath = args.getString("reference");
        outputPath = args.getString("output");
    } catch (std::invalid_argument const& e) {
        std::cerr << "Usage: " << argv[0] << " reference output <?--preftab>\n";
        std::exit(1);
    }


    auto suffixArray = suffixarray::SuffixArray::fromFASTAFile(referencePath, preftab);
    suffixArray.save(outputPath);

    std::cout << suffixArray.data().size() << "," << preftab << "," << suffixArray.getSuffixArrayBuildTime()
        << "," << suffixArray.getPrefixTableBuildTime() << "," << std::filesystem::file_size(outputPath) << "\n";
}