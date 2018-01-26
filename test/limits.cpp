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

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    pyint.set_execution_step_limit(5);

    ASSERT_THROW(pyint.execute(), execution_limit_exception);
}

TEST(Limits, below_limit)
{
    const std::string code = "for _ in range(10):\n"
                             "  pass";

    auto doc = compile_code(code);

    Interpreter pyint(doc);
    pyint.set_execution_step_limit(30);
    pyint.execute();
}

TEST(Limits, out_of_memory)
{
    const std::string code = "a = []\n"
                             "for _ in range(10000):\n"
                             "   a.append('somestring')";

    auto doc = compile_code(code);

    Interpreter pyint(doc);

    ASSERT_THROW(pyint.execute(), execution_limit_exception);
}


}
