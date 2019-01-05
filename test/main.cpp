#include <gtest/gtest.h>

void print_program_output(const std::string &str){
    // no output in tests
}

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
