#include <cowlang/cow.h>
#include <cowlang/unpack.h>

#include <gtest/gtest.h>

using namespace cow;

class TupleTest : public testing::Test
{
};

TEST(TupleTest, access_element)
{
    const std::string code = "v = ('a', 1, 'k')\n"
                             "return v[2]";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    std::string expected = "k";
    EXPECT_EQ(expected, unpack_string(res));
}
