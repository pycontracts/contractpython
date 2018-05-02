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

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    EXPECT_EQ(3, unpack_integer(pyint.execute()));
}

TEST(BasicTest, arraylen)
{
    const std::string code = "a = ['foo', 'bar']\n"
                             "return len(a)";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    EXPECT_EQ(2, unpack_integer(pyint.execute()));
}

TEST(BasicTest, arraystr)
{
    const std::string code = "a = ['foo', 'bar']\n"
                             "return str(a)";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    EXPECT_EQ("['foo', 'bar']", unpack_string(pyint.execute()));
}
TEST(BasicTest, pass)
{
    const std::string code = "pass\n"
                             "return True";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    EXPECT_TRUE(unpack_bool(pyint.execute()));
}

TEST(BasicTest, greater_than)
{
    const std::string code = "i = 0\n"
                             "return i > -1";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    EXPECT_TRUE(unpack_bool(pyint.execute()));
}

TEST(BasicTest, while_loop)
{
    const std::string code =
            "i = 0\n"
            "while i < 3:\n"
            "   i += 1\n"
            "return i == 3";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, rand)
{
    const std::string code =
            "import rand\n"
            "r = rand.randint(0,10)\n"
            "return r >= 0 and r <= 10\n";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, rand2)
{
    const std::string code =
            "from rand import randint\n"
            "r = randint(0,10)\n"
            "return r >= 0 and r <= 10\n";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, array)
{
    const std::string code =
            "arr = [5,4,1337,2]\n"
            "if arr[2] == 1337:\n"
            "	return True\n"
            "return False";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, array_append)
{
    const std::string code =
            "arr = [5,4]\n"
            "arr.append(1337)\n"
            "arr.append(2)\n"
            "if arr[2] == 1337:\n"
            "	return True\n"
            "return False";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}
TEST(BasicTest, dictionary)
{
    const std::string code =
            "i = {'value':42}\n"
            "return i['value']";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    auto res = pyint.execute();
    EXPECT_EQ(unpack_integer(res), 42);
}

TEST(BasicTest, if_none)
{
    const std::string code =
            "i = None\n"
            "if i:\n"
            "	return False\n"
            "\n"
            "return True";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, if_else_none)
{
    const std::string code =
            "i = None\n"
            "if i:\n"
            "	return False\n"
            "else:\n"
            "	return True";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}



TEST(BasicTest, if_clause)
{
    const std::string code =
            "i = 42\n"
            "if i == 43:\n"
            "	return False\n"
            "else:\n"
            "	return True";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, str_eq)
{
    const std::string code =
            "a = 'foo'\n"
            "b = 'foo'\n"
            "eq = (a == b)\n"
            "return eq";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, none_value)
{
    const std::string code =
            "a = None\n"
            "if not a:\n"
            "   return False\n"
            "return True";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_FALSE(unpack_bool(res));
}

TEST(BasicTest, logical_and)
{
    const std::string code =
            "a = False\n"
            "b = True\n"
            "return (not a) and b";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, create_by_assign)
{
    const std::string code =
            "a = 1\n"
            "b = a+1\n"
            "return b";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_EQ(unpack_integer(res), 2);
}

TEST(BasicTest, iterate_dict)
{
    const std::string code =
           "res  = 0\n"
           "dict = {'a':1, 'b':2}\n"
           "for t in dict.items():\n"
           "    k,v = t\n"
           "    if k == 'b':\n"
           "       res = v\n"
           "return res == 2";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, or_op)
{
    const std::string code =
           "dict = None\n"
           "if not dict or dict['b'] == 1:\n"
           "    return True\n"
           "else:\n"
           "    return False";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, iterate_dict2)
{
    const std::string code =
           "res  = 0\n"
           "dict = {'a':1, 'b':2}\n"
           "for k,v in dict.items():\n"
           "    if k == 'b':\n"
           "       res = v\n"
           "return res == 2";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, not)
{
    const std::string code =
            "b = False\n"
            "return not b";

    auto doc = compile_code(code);
    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, pre_set_list)
{
    const std::string code =
            "return b[1] == 'foo'";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    pyint.set_list("b", {"bar", "foo"});
    auto res = pyint.execute();

    EXPECT_TRUE(unpack_bool(res));
}

TEST(BasicTest, set_variable)
{
    const std::string code =
            "b = False \n"
            "return b";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    auto res = pyint.execute();

    EXPECT_FALSE(unpack_bool(res));
}

TEST(BasicTest, document_to_value)
{
    DummyMemoryManager mem;
    
    json::Document doc("{ \"a\": 1, \"b\": 2}");

    auto val = mem.create_from_document(doc);

    auto dic = value_cast<Dictionary>(val);

    EXPECT_EQ(dic->size(), 2);
}

TEST(BasicTest, pre_set_value)
{
    std::string code = "if op_type == 'put':\n"
                      "    return False\n"
                      "else:\n"
                      "	   return True";

    auto data = compile_code(code);
    Interpreter interpreter(data);

    interpreter.set_string("op_type", "put");

    auto res = interpreter.execute();
    EXPECT_FALSE(unpack_bool(res));
}
