#include <json/json.h>
#include <json/Document.h>

#include <gtest/gtest.h>

using namespace json;

class WriterTest : public testing::Test
{
};

TEST(WriterTest, empty)
{
    Writer writer1;
    auto doc1 = writer1.make_document();

    Writer writer2;
    writer2.write_null("");

    auto doc2 = writer2.make_document();

    EXPECT_TRUE(doc1.empty());
    EXPECT_TRUE(doc2.empty());

    EXPECT_TRUE(doc1.valid());
    EXPECT_TRUE(doc2.valid());
}

