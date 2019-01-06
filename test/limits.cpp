#include <cowlang/cow.h>

#include <gtest/gtest.h>

namespace cow
{

class Limits : public ::testing::Test
{
};

TEST(Limits, above_limit)
{
    const std::string code = "for _ in range(10):\n"
                             "  pass";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    pyint.set_execution_step_limit(5);

    ASSERT_THROW(pyint.execute(), execution_limit_exception);
}

TEST(Limits, below_limit)
{
    const std::string code = "for _ in range(10):\n"
                             "  pass";

    auto doc = compile_string(code);

    DummyMemoryManager mem;
    Interpreter pyint(doc, mem);

    pyint.set_execution_step_limit(30);
    pyint.execute();
}

/*
TEST(Limits, out_of_memory)
{
    /// TODO
    /// currently this just tries to allocate more then a page size
    /// Once we have paging implement setting memory size limits
    const std::string code = "a = []\n"
                             "for _ in range(100000):\n"
                             "   a.append('somestring')";

    auto doc = compile_string(code);

    Interpreter pyint(doc);

    ASSERT_THROW(pyint.execute(), execution_limit_exception);
}*/

} // namespace cow
