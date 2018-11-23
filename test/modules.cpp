#include <cowlang/cow.h>
#include <cowlang/unpack.h>

#include <gtest/gtest.h>

using namespace cow;

class ModuleTest : public testing::Test
{
};


class FooObj : public Module
{
public:
    using Module::Module;

    ValuePtr get_member(const std::string &name)
    {
        auto &mem = memory_manager();

        return make_value<Function>(mem,
              [&](const std::vector<ValuePtr> &args) -> ValuePtr {
                    return wrap_value(new (mem) IntVal(mem, 42));
        });
    }
};

class Foo2Obj : public Module
{
public:
    using Module::Module;

    ValuePtr get_member(const std::string &name)
    {
        auto &mem = memory_manager();

        return make_value<Function>(mem, 
              [&](const std::vector<ValuePtr> &args) -> ValuePtr {
                  auto i = value_cast<IntVal>(args[0]);
                  return wrap_value(new (mem) IntVal(mem,i->get() * 2));
        });
    }
};

TEST(ModuleTest, call_cpp_with_argument)
{
    const std::string code =
            "from foo import double\n"
            "f = double(21)\n"
            "if f == 42:\n"
            "	return True\n"
            "else:\n"
            "	return False";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    auto v = wrap_value(new (pyint.memory_manager()) Foo2Obj(pyint.memory_manager()));
    pyint.set_module("foo", v);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}

TEST(ModuleTest, call_cpp)
{
    const std::string code =
            "import foo\n"
            "f = foo.get()\n"
            "if f == 42:\n"
            "	return True\n"
            "else:\n"
            "	return False";

    auto doc = compile_string(code);
    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);
    
    auto v = wrap_value(new (pyint.memory_manager()) FooObj(pyint.memory_manager()));
    pyint.set_module("foo", v);

    auto res = pyint.execute();
    EXPECT_TRUE(unpack_bool(res));
}


