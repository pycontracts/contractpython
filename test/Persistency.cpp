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
                             "store['test'] = 3\n"
                             "return store['test']";

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
                             "store[test] = 3\n"
                             "return store['test']";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);
    register_blockchain_module(pyint);
    ASSERT_THROW(pyint.execute(), std::exception);
}

TEST(PersistencyTest, dict_integer_indices_are_forbidden)
{
    const std::string code = "import blockchain as b\n"
                             "store[5] = 3\n"
                             "return store[5]";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);
    register_blockchain_module(pyint);
    ASSERT_THROW(pyint.execute(), std::exception);
}
TEST(PersistencyTest, dict_integer_indices_are_forbidden_2)
{
    const std::string code = "import blockchain as b\n"
                             "store['5'] = 3\n"
                             "return store[5]";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);
    register_blockchain_module(pyint);
    ASSERT_THROW(pyint.execute(), std::exception);
}

TEST(PersistencyTest, super_bad_nonstring_subscript_with_is_also_nontranslatable)
{
    const std::string code = "import blockchain as b\n"
                             "store[te st] = 3\n"
                             "return store['te st']";

    ASSERT_THROW(compile_string(code), std::exception);
}

TEST(PersistencyTest, super_bad_nonstring_subscript_on_nonarray)
{
    const std::string code = "import blockchain as b\n"
                             "store[2] = 3\n"
                             "return store[2]";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);
    register_blockchain_module(pyint);
    ASSERT_THROW(pyint.execute(), std::exception);
}
