#include <cowlang/cow.h>
#include <cowlang/unpack.h>
#include <gtest/gtest.h>
#include <modules/blockchain_module.h>

using namespace cow;

class PersistencyTest : public testing::Test
{
};

TEST(PersistencyTest, simple_case)
{
    const std::string code = "import blockchain as b\n"
                             "b.store['test'] = 3\n"
                             "return b.store['test']";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);
    register_blockchain_module(pyint);
    auto res = pyint.execute();
    EXPECT_EQ(3, unpack_integer(res));
}


TEST(PersistencyTest, super_bad_nonstring_subscript)
{
    const std::string code = "import blockchain as b\n"
                             "b.store[test] = 3\n"
                             "return b.store['test']";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);
    register_blockchain_module(pyint);
    ASSERT_THROW(pyint.execute(), std::exception);
}
