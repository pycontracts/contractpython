#include <json/json.h>
#include <json/Document.h>

#include <gtest/gtest.h>

using namespace json;

class Search : public testing::Test
{
};

TEST(Search, root_view)
{
    Document doc("{\"a\" :1}");
    Document view(doc, "");

    EXPECT_TRUE(doc == view);
}

TEST(Search, non_view)
{
    Document doc("{\"a\" :1}");
    Document view(doc, "b", false);

    EXPECT_FALSE(view.valid());
}

TEST(Search, non_view_except)
{
    Document doc("{\"a\" :1}");

    EXPECT_THROW(Document(doc, "b", true), json_error);
}

TEST(Search, filter)
{
    Document doc("{\"a\":41, \"b\":42, \"c\": 42}");

    Document filtered(doc, std::vector<std::string>{"b"});

    EXPECT_EQ(filtered.str(), "{\"b\":42}");
}

TEST(Search, wildcard_filter)
{
    Document doc("{\"a\":[{\"b\":41,\"c\":42},{\"b\":43,\"c\":42}]}");

    Document filtered(doc, std::vector<std::string>{"a.*.b"});

    EXPECT_EQ(filtered.str(), "{\"a\":[{\"b\":41},{\"b\":43}]}");
}

TEST(Search, wildcard_filter_nested)
{
    Document doc("{\"a\":[{\"b\":{\"c\":42}}]}");

    Document filtered(doc, std::vector<std::string>{"a.*.b.c"});

    EXPECT_EQ(filtered.str(), "{\"a\":[{\"b\":{\"c\":42}}]}");
}


