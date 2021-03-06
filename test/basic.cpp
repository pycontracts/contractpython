#include <cowlang/cow.h>
#include <cowlang/unpack.h>

#include <gtest/gtest.h>

using namespace cow;

class BasicTest : public testing::Test
{
};

TEST(BasicTest, strlen)
{
    const std::string code = "s = 'foo'\n"
                             "return len(s)";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(3, unpack_integer(pyint.execute()));
}

TEST(BasicTest, arraylen)
{
    const std::string code = "a = ['foo', 'bar']\n"
                             "return len(a)";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ(2, unpack_integer(pyint.execute()));
}

TEST(BasicTest, arraystr)
{
    const std::string code = "a = ['foo', 'bar']\n"
                             "return str(a)";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ("['foo', 'bar']", unpack_string(pyint.execute()));
}

TEST(BasicTest, arrayassignment)
{
    const std::string code = "a = [1,2,3]\n"
                             "a[0] = 'loop'\n"
                             "return str(a[0])";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ("loop", unpack_string(pyint.execute()));
}

TEST(BasicTest, dictassignment)
{
    const std::string code = "a = {'a':'foo', 'b':'bar'}\n"
                             "a['b'] = 'loop'\n"
                             "return str(a['b'])";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ("loop", unpack_string(pyint.execute()));
}

TEST(BasicTest, dictassignment_in_initial_assign_wrong_index)
{
    const std::string code = "a = {5:'foo', 'b':'bar'}";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);
    ASSERT_THROW(pyint.execute(), std::exception);
}

TEST(BasicTest, dictget)
{
    const std::string code = "a = {'a':'foo', 'b':'bar'}\n"
                             "return a['b']";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ("bar", unpack_string(pyint.execute()));
}

TEST(BasicTest, multiwordstring)
{
    const std::string code = "a = \"hello fucking world\"\n"
                             "return str(a)";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ("hello fucking world", unpack_string(pyint.execute()));
}

TEST(BasicTest, longstring)
{
    const std::string code = "a = "
                             "\"aabbccddeeffgghhaabbccddeeffgghhaabbccddeeffgghhaabbccddeeffgghhaab"
                             "bccddeeffgghhaabbccddeeffgghhaabbccddeeffgghhaabbccddeeffgghhaabbccdd"
                             "eeffgghhaabbccddeeffgghhaabbccddeeffgghh\"\n"
                             "return str(a)";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_EQ("aabbccddeeffgghhaabbccddeeffgghhaabbccddeeffgghhaabbccddeeffgghhaab"
              "bccddeeffgghhaabbccddeeffgghhaabbccddeeffgghhaabbccddeeffgghhaabbccdd"
              "eeffgghhaabbccddeeffgghhaabbccddeeffgghh",
              unpack_string(pyint.execute()));
}

TEST(BasicTest, pass)
{
    const std::string code = "pass\n"
                             "return True";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_TRUE(unpack_bool(pyint.execute()));
}

TEST(BasicTest, greater_than)
{
    const std::string code = "i = 0\n"
                             "return i > -1";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    EXPECT_TRUE(unpack_bool(pyint.execute()));
}

TEST(BasicTest, while_loop)
{
    const std::string code = "i = 0\n"
                             "while i < 3:\n"
                             "   i += 1\n"
                             "return i == 3";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, rand)
{
    const std::string code = "import rand\n"
                             "r = rand.randint(0,10)\n"
                             "return r >= 0 and r <= 10\n";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, rand2)
{
    const std::string code = "from rand import randint\n"
                             "r = randint(0,10)\n"
                             "return r >= 0 and r <= 10\n";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, array)
{
    const std::string code = "arr = [5,4,1337,2]\n"
                             "if arr[2] == 1337:\n"
                             "	return True\n"
                             "return False";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, array_append)
{
    const std::string code = "arr = [5,4]\n"
                             "arr.append(1337)\n"
                             "arr.append(2)\n"
                             "if arr[2] == 1337:\n"
                             "	return True\n"
                             "return False";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}
TEST(BasicTest, dictionary)
{
    const std::string code = "i = {'value':42}\n"
                             "return i['value']";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_EQ(unpack_integer(res), 42);
}

TEST(BasicTest, if_none)
{
    const std::string code = "i = None\n"
                             "if i:\n"
                             "	return False\n"
                             "\n"
                             "return True";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, if_else_none)
{
    const std::string code = "i = None\n"
                             "if i:\n"
                             "	return False\n"
                             "else:\n"
                             "	return True";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}


TEST(BasicTest, if_clause)
{
    const std::string code = "i = 42\n"
                             "if i == 43:\n"
                             "	return False\n"
                             "else:\n"
                             "	return True";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, str_eq)
{
    const std::string code = "a = 'foo'\n"
                             "b = 'foo'\n"
                             "eq = (a == b)\n"
                             "return eq";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, none_value)
{
    const std::string code = "a = None\n"
                             "if not a:\n"
                             "   return False\n"
                             "return True";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_FALSE(unpack_bool(res));
}

TEST(BasicTest, logical_and)
{
    const std::string code = "a = False\n"
                             "b = True\n"
                             "return (not a) and b";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);
    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, create_by_assign)
{
    const std::string code = "a = 1\n"
                             "b = a+1\n"
                             "return b";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_EQ(unpack_integer(res), 2);
}

TEST(BasicTest, range2)
{
    const std::string code = "i = 0\n"
                             "for j in range(4,10):\n"
                             "   i += j\n"
                             "return i";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_EQ(39, unpack_integer(res));
}

TEST(BasicTest, range3)
{
    const std::string code = "i = 0\n"
                             "for j in range(1,5,2):\n"
                             "   i += j\n"
                             "return i";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_EQ(4, unpack_integer(res));
}

TEST(BasicTest, iterate_dict)
{
    const std::string code = "res  = 0\n"
                             "dict = {'a':1, 'b':2}\n"
                             "for t in dict.items():\n"
                             "    k,v = t\n"
                             "    if k == 'b':\n"
                             "       res = v\n"
                             "return res == 2";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, or_op)
{
    const std::string code = "dict = None\n"
                             "if not dict or dict['b'] == 1:\n"
                             "    return True\n"
                             "else:\n"
                             "    return False";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, iterate_dict2)
{
    const std::string code = "res  = 0\n"
                             "dict = {'a':1, 'b':2}\n"
                             "for k,v in dict.items():\n"
                             "    if k == 'b':\n"
                             "       res = v\n"
                             "return res == 2";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, not)
{
    const std::string code = "b = False\n"
                             "return not b";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, pre_set_list)
{
    const std::string code = "return b[1] == 'foo'";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    pyint.set_list("b", { "bar", "foo" });
    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, set_variable)
{
    const std::string code = "b = False \n"
                             "return b";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);
    auto res = pyint.execute();

    EXPECT_FALSE(unpack_bool(res));
}


TEST(BasicTest, pre_set_value)
{
    std::string code = "if op_type == 'put':\n"
                       "    return False\n"
                       "else:\n"
                       "	   return True";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    pyint.set_string("op_type", "put");

    auto res = pyint.execute();
    EXPECT_FALSE(unpack_bool(res));
}
