#include <cowlang/cow.h>
#include <cowlang/unpack.h>

#include <gtest/gtest.h>

using namespace cow;

class LoopTest : public testing::Test
{
};

TEST(LoopTest, skip_while_loop)
{
    const std::string code = "return True\n"
                             "while False:\n"
                             "   pass\n"
                             "return False";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(LoopTest, for_loop)
{
    const std::string code = "l = [1,2,3]\n"
                             "res = 0\n"
                             "for i in l:\n"
                             "   res += i\n"
                             "return res == 6";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(LoopTest, while_bool_cond)
{
    const std::string code = "v = False\n"
                             "while not v:\n"
                             "  v = True\n"
                             "return v";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(LoopTest, while_loop_break1)
{
    const std::string code =
           "a = 5\n"
           "while True:\n"
           "    a += 1\n"
           "    break\n"
           "return a == 6";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(LoopTest, while_loop_break2)
{
    const std::string code =
           "a = 5\n"
           "while True:\n"
           "    a += 1\n"
           "    break\n"
           "    a += 5\n"
           "return a == 6";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(LoopTest, while_loop_return)
{
    const std::string code =
           "a = 5\n"
           "while True:\n"
           "    a += 1\n"
           "    return a\n"
           "    a += 5\n"
           "return a";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_EQ(6, unpack_integer(res));
}

TEST(LoopTest, for_loop_break)
{
    const std::string code =
           "a = 5\n"
           "for _ in range(10):\n"
           "    a += 1\n"
           "    break\n"
           "return a == 6";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(LoopTest, for_loop_return)
{
    const std::string code =
           "for _ in range(10):\n"
           "    return True\n"
           "return False";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(LoopTest, loop_continue)
{
    const std::string code =
           "a = 5\n"
           "for _ in range(10):\n"
           "    continue\n"
           "    a += 1\n"
           "return a == 5";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(LoopTest, array_loop)
{
    const std::string code = 
        "a = [2*i for i in range(5)]\n"
        "return a[3]";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_EQ(6, unpack_integer(res));
}


