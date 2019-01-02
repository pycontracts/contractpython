#include <json/json.h>
#include <json/Document.h>

#include <gtest/gtest.h>

using namespace json;

class Basic : public testing::Test
{
};

TEST(Basic, empty)
{
    json::Document invalid;
    json::Document doc("");

    EXPECT_FALSE(invalid.valid());
    EXPECT_TRUE(doc.valid());
    EXPECT_TRUE(doc.empty());
}

TEST(Basic, invalid)
{
    uint8_t garbage[42];
    garbage[0] = static_cast<uint8_t>(ObjectType::Null);

    json::Document invalid(garbage, 42, DocumentMode::ReadOnly);
    EXPECT_FALSE(invalid.valid());
}

TEST(Basic, spaces)
{
    Document doc1("{\"a\" :1}");
    Document doc2("{\"a\": 1}");

    EXPECT_TRUE(doc1 == doc2);
}

TEST(Basic, compress_two)
{
    Document input1("{\"a\":1}");
    Document input2("{\"a\":2}");

    bitstream bstream;
    input1.compress(bstream);
    input2.compress(bstream);

    bstream.move_to(0);
    Document output1(bstream);
    Document output2(bstream);

    EXPECT_EQ(input1.str(), output1.str());
    EXPECT_EQ(input2.str(), output2.str());
}

TEST(Basic, binary)
{
    constexpr uint32_t length = 1235;
    uint8_t value[length];

    json::Binary bin(value, length);

    json::Document doc("{\"a\":[]}");
    doc.insert("a.+", bin);

    json::Document view(doc, "a.0");

    EXPECT_EQ(memcmp(view.as_bitstream().data(), value, length), 0);
}

TEST(Basic, binary2)
{
    bitstream bs;
    bs << "This is only a test";

    json::Binary bin(bs);

    json::Document doc("{\"a\":[]}");
    doc.insert("a.+", bin);

    json::Document view(doc, "a.0");

    EXPECT_EQ(view.as_bitstream(), bs);
}

TEST(Basic, datetime)
{
    std::string str = "d\"1955-11-05 12:00:00\"";
    Document input(str);

    bitstream bstream;
    input.compress(bstream);

    bstream.move_to(0);

    Document output(bstream);
    EXPECT_EQ(input.str(), output.str());
    EXPECT_EQ(input.str(), str);
}

TEST(Basic, boolean)
{
    Document input("true");
    EXPECT_EQ(input.str(), "true");
}

TEST(Basic, map)
{
    Document doc("{\"a\":\"b\", \"c\":1}");

    bitstream bstream;
    doc.compress(bstream);

    bstream.move_to(0);
    Document doc2(bstream);

    EXPECT_EQ(doc.str(), "{\"a\":\"b\",\"c\":1}");
    EXPECT_EQ(doc2.str(), "{\"a\":\"b\",\"c\":1}");
}

TEST(Basic, array)
{
    Document doc("[1,2,3]");

    bitstream bstream;
    doc.compress(bstream);

    bstream.move_to(0);
    Document doc2(bstream);

    EXPECT_EQ(doc2.str(), doc.str());
}

TEST(Basic, nested_map)
{
    Document doc("{\"a\":{\"b\":42}}");

    bitstream bstream;
    doc.compress(bstream);

    bstream.move_to(0);
    Document doc2(bstream);

    EXPECT_EQ(doc2.str(), doc.str());
}

TEST(Basic, array_in_map)
{
    Document doc("{\"a\":[\"b\",42]}");

    bitstream bstream;
    doc.compress(bstream);

    bstream.move_to(0);
    Document doc2(bstream);

    EXPECT_EQ(doc2.str(), doc.str());
}

TEST(Basic, cannot_append_to_non_array)
{
    Document doc("{\"a\":[4,3,2], \"b\":{}}");
    bool result = doc.insert("b.+", json::Document("23"));

    EXPECT_FALSE(result);
    EXPECT_EQ(doc.str(), "{\"a\":[4,3,2],\"b\":{}}");
}

TEST(Basic, array_nested_multi_append)
{
    Document doc("{\"b\":\"xyz\",\"a\":{\"foo\":[4,3,2]}, \"bar\": 1337}");

    for(auto i = 0; i < 5; ++i)
    {
        doc.insert("a.foo.+", json::Document("23"));
    }

    EXPECT_EQ(doc.str(), "{\"b\":\"xyz\",\"a\":{\"foo\":[4,3,2,23,23,23,23,23]},\"bar\":1337}");
}

TEST(Basic, array_nested_pppend)
{
    Document doc("{\"b\":\"xyz\",\"a\":{\"foo\":[4,3,2]}, \"bar\": 1337}");
    bool result = doc.insert("a.foo.+", json::Document("23"));

    EXPECT_TRUE(result);
    EXPECT_EQ(doc.str(), "{\"b\":\"xyz\",\"a\":{\"foo\":[4,3,2,23]},\"bar\":1337}");
}

TEST(Basic, array_append)
{
    Document doc("{\"b\":\"xyz\",\"a\":[4,3,2]}");
    bool result = doc.insert("a.+", json::Document("23"));

    EXPECT_TRUE(result);
    EXPECT_EQ(doc.str(), "{\"b\":\"xyz\",\"a\":[4,3,2,23]}");
}

TEST(Basic, update)
{
    Document doc("{\"a\":42}");
    doc.insert("a", json::Document("23"));
    EXPECT_EQ(doc.str(), "{\"a\":23}");
}

TEST(Basic, get_key)
{
    Document doc("{\"a\":42, \"b\": \"foobar\"}");
   
    EXPECT_EQ(doc.get_size(), 2);
    EXPECT_EQ(doc.get_key(1), "b");
}

TEST(Basic, get_child)
{
    Document doc("{\"a\":42, \"b\": \"foobar\"}");
   
    EXPECT_EQ(doc.get_size(), 2);
    EXPECT_EQ(doc.get_child(1).as_string(), "foobar");
}

TEST(Basic, add)
{
    Document doc("{\"a\":42}");
    Document to_add("5");

    bool result = doc.add("a", to_add);

    EXPECT_TRUE(result);
    EXPECT_EQ(doc.str(), "{\"a\":47}");
}

TEST(Basic, insert)
{
    Document doc("{\"a\":42}");
    bool result = doc.insert("b", json::Document("23"));

    EXPECT_TRUE(result);
    EXPECT_EQ(doc.str(), "{\"a\":42,\"b\":23}");
}

TEST(Basic, view)
{
    Document doc("{\"a\":{\"b\":[1,2,4,5]}}");
    Document view(doc, "a.b");

    EXPECT_EQ(view.str(), "[1,2,4,5]");
}

TEST(Basic, array_view)
{
    Document doc("{\"a\":[1,2,{\"b\":[42,23]},5]}}");
    Document view(doc, "a.2.b");

    EXPECT_EQ(view.str(), "[42,23]");
}

TEST(Basic, view2)
{
    Document doc("{\"a\":{\"b\":[1,2,4,5]},\"c\":42}");

    Document view(doc, "c");
    EXPECT_EQ(view.str(), "42");
}

TEST(Basic, no_diffs)
{
    Document doc1("{\"a\":42}");
    Document doc2("{\"a\":42}");

    auto diffs = doc1.diff(doc2);
    EXPECT_EQ(diffs.size(), 0);
}

TEST(Basic, diffs)
{
    Document doc1("{\"a\":1}");
    Document doc2("{\"a\":2}");

    auto diffs = doc1.diff(doc2);

    EXPECT_EQ(diffs.size(), 1);
    EXPECT_EQ(diffs.begin()->as_document(), json::Document("{\"type\":\"modified\",\"path\":\"a\",\"new_value\":2}"));
}

TEST(Basic, compress_diffs)
{
    Document doc1("{\"a\":1}");
    Document doc2("{\"a\":2}");

    auto diffs = doc1.diff(doc2);

    bitstream bstream;
    diffs.begin()->compress(bstream, true);

    bstream.move_to(0);

    json::Document doc(bstream);
    EXPECT_EQ(doc, json::Document("{\"type\":\"modified\",\"path\":\"a\",\"new_value\":2}"));
}

TEST(Basic, diffs_str)
{
    Document doc1("{\"a\":\"here\"}");
    Document doc2("{\"a\":\"we go\"}");

    auto diffs = doc1.diff(doc2);

    EXPECT_EQ(diffs.size(), 1);
    EXPECT_EQ(diffs.begin()->as_document(), json::Document("{\"type\":\"modified\",\"path\":\"a\",\"new_value\":\"we go\"}"));
}

TEST(Basic, diff_deleted)
{
    Document doc1("{\"a\":\"here\"}");
    Document doc2("{}");

    auto diffs = doc1.diff(doc2);

    EXPECT_EQ(diffs.size(), 1);
    EXPECT_EQ(diffs.begin()->as_document(), json::Document("{\"type\":\"deleted\",\"path\":\"a\"}"));
}

TEST(Basic, diff_added)
{
    Document doc1("{\"a\":\"here\"}");
    Document doc2("{\"a\":\"here\",\"b\":\"we go\"}");

    auto diffs = doc1.diff(doc2);

    EXPECT_EQ(diffs.size(), 1);
    EXPECT_EQ(diffs.begin()->as_document(), json::Document("{\"type\":\"added\",\"path\":\"b\",\"value\":\"we go\"}"));
}

TEST(Basic, diff_added_array)
{
    Document doc1("[\"here\"]");
    Document doc2("[\"here\",\"we go\"]");

    auto diffs = doc1.diff(doc2);

    EXPECT_EQ(diffs.size(), 1);
    EXPECT_EQ(diffs.begin()->as_document(), json::Document("{\"type\":\"added\",\"path\":\"1\",\"value\":\"we go\"}"));
}

TEST(Basic, cant_put_path)
{
    json::String doc("str");

    bool res = doc.insert("foo", json::Integer(42));

    EXPECT_FALSE(res);
    EXPECT_EQ(doc.str(), "\"str\"");
}

TEST(Basic, match_predicates)
{
    json::Document doc("{\"NO_D_ID\":1,\"NO_W_ID\":4,\"NO_O_ID\":300}");
    json::Document predicates("{\"NO_D_ID\":1,\"NO_W_ID\":4}");

    EXPECT_TRUE(doc.matches_predicates(predicates));
}
