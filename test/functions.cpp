#include <cowlang/cow.h>
#include <cowlang/unpack.h>
#include <gtest/gtest.h>

namespace cow
{

class Functions : public ::testing::Test
{
};

TEST(Functions, simple_call)
{
    const std::string code = "def test():\n"
                             "  return 1\n"
                             "return test()";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(1, unpack_integer(pyint.execute()));
}

TEST(Functions, argument_call)
{
    const std::string code = "def test(a,b):\n"
                             "  return a*3+b\n"
                             "return test(3,3)";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(12, unpack_integer(pyint.execute()));
}

TEST(Functions, argument_call_last_is_default)
{
    const std::string code = "def test(a,b=3):\n"
                             "  return a*3+b\n"
                             "return test(3)";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(12, unpack_integer(pyint.execute()));
}

TEST(Functions, argument_call_last_is_default_override)
{
    const std::string code = "def test(a,b=3):\n"
                             "  return a*3+b\n"
                             "return test(3,4)";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(13, unpack_integer(pyint.execute()));
}

TEST(Functions, argument_call_first_is_default_second_not)
{
    const std::string code = "def test(a=3,b):\n"
                             "  return a*3+b\n"
                             "return test(3,4)";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    ASSERT_THROW(pyint.execute(), std::exception);
}


TEST(Functions, argument_not_altering_global_scope)
{
    const std::string code = "tester = 1\n"
                             "def test(a=3,b=3):\n"
                             "  tester = 3\n"
                             "  return a*3+b\n"
                             "test(1,1)\n"
                             "return tester";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(1, unpack_integer(pyint.execute()));
}

TEST(Functions, argument_not_altering_global_scope_but_inner_scope)
{
    const std::string code = "tester = 1\n"
                             "def test(a=3,b=3):\n"
                             "  tester = 3\n"
                             "  return tester\n"
                             "return test(1,1) + tester";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(4, unpack_integer(pyint.execute()));
}


TEST(Functions, func_can_read_global_scope)
{
    const std::string code = "tester = 1\n"
                             "def test(a=3,b=3):\n"
                             "  return tester\n"
                             "return test(1,2)";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(1, unpack_integer(pyint.execute()));
}

TEST(Functions, argument_altering_global_scope_with_global_keyword)
{
    const std::string code = "tester = 1\n"
                             "def test(a=3,b=3):\n"
                             "  global tester\n"
                             "  tester = 3\n"
                             "  return a*3+b\n"
                             "test()\n"
                             "return tester";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(3, unpack_integer(pyint.execute()));
}

TEST(Functions, argument_altering_global_scope_with_global_keyword_no_prior_assignment)
{
    const std::string code = "def test(a=3,b=3):\n"
                             "  global tester\n"
                             "  tester = 3\n"
                             "  return a*3+b\n"
                             "test()\n"
                             "return tester";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(3, unpack_integer(pyint.execute()));
}

TEST(Functions, nested_scopes)
{
    const std::string code = "tester = 1\n"
                             "def test(a=3,b=3):\n"
                             "  tester = 2\n"
                             "  def test2():\n"
                             "    return tester\n"
                             "  return test2()\n"
                             "return test(1,2)";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(2, unpack_integer(pyint.execute()));
}

}
