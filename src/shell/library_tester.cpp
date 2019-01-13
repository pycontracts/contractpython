#include <cowlang/library.h>
#include <fstream>
#include <iostream>
int main()
{
    init_cryptopython();
    uint64_t used_g = 0;
    blockchain_arguments args = {
        "a1075db55d416d3ca199f55b6084e2115b9345e16c5cf302fc80e9d5fbf5d48d",
        "00000000000000000015c23c0979270b91c26a562ae62463b85481f1d945bc21",
        "0000000000000000002f3cb3939d8685c8976dc9e35ccec08c4e121b12688974",
        1546659311,
        1546659307,
        "1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa",
        "1XPTgDRhN8RFnzniWCddobD9iKZatrvH4",
        (uint64_t)50 * COIN,
        (uint64_t)300 * COIN
    };
    std::ifstream input("librarytest.py.bitstream", std::ios::binary);
    std::string str((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    std::stringstream ss;
    std::string os = "";
    execute_program(str, cow::net_type::MAIN, args, 50000000, 100, used_g, os, ss);
    std::cout << "Program returned:" << std::endl << get_outbuf() << std::endl;
    std::cout << "Errors were:" << std::endl << get_errorbuf() << std::endl;
    std::cout << "Gas used: " << used_g << std::endl;
}
