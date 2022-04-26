/**
 * @file tests.cc
 * @author Daniel Nichols
 * @brief 
 * @date 2022-04-18
 */

#include <iostream>

#include "suffixarray.h"

constexpr void ASSERT_EQUAL(auto a, auto b, std::string const& msg) {
    if (a != b) {
        std::cerr << msg << std::endl;
        std::exit(1);
    }
}

void test_loadsave();

int main() {
    test_loadsave();

    std::cout << "Tests successful!" << std::endl;
}

void test_loadsave() {
    using namespace suffixarray;

    {   // load file -- save -- reload compare -- NO prefixtable
        SuffixArray sa1 = SuffixArray::fromFASTAFile("inputs/banana.fa");
        sa1.save("tmp.sa");
        SuffixArray sa2 = SuffixArray::fromSave("tmp.sa");
        std::remove("tmp.sa");
        
        ASSERT_EQUAL(sa1.data(), sa2.data(), "Data not equal after load.");
        ASSERT_EQUAL(sa1.suffixes(), sa2.suffixes(), "Suffixes not equal after load.");
    }
}