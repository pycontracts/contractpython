#include <json/json.h>
#include <json/Document.h>

#include <gtest/gtest.h>

using namespace json;

class PredicatesTest : public testing::Test
{
};

TEST(PredicatesTest, root_predicate)
{
    Integer i(5);

    Document predicate1("{\"$lte\":5}");
    Document predicate2("{\"$eq\": 3}");
    Document predicate3("{\"$lt\":9}");
 
    EXPECT_TRUE(i.matches_predicates(predicate1));
    EXPECT_FALSE(i.matches_predicates(predicate2));
    EXPECT_TRUE(i.matches_predicates(predicate3));
}

TEST(PredicatesTest, float_int_redicate)
{
    Document doc("{\"a\": 5.2}");

    Document predicate1("{\"a\" :{\"$lte\":5.5}}");
    Document predicate2("{\"a\" :{\"$eq\": 3}}");
    Document predicate3("{\"a\" :{\"$gte\": 3}}");
    Document predicate4("{\"a\" :{\"$eq\":5.2}}");
    Document predicate5("{\"a\" :{\"$neq\":5.2}}");
 
    EXPECT_TRUE(doc.matches_predicates(predicate1));
    EXPECT_FALSE(doc.matches_predicates(predicate2));
    EXPECT_TRUE(doc.matches_predicates(predicate3));
    EXPECT_TRUE(doc.matches_predicates(predicate4));
    EXPECT_FALSE(doc.matches_predicates(predicate5));
}

TEST(PredicatesTest, string_predicate)
{
    Document doc("{\"a\": \"foobar\"}");

    Document predicate1("{\"a\" :{\"$in\": [\"xyz\", \"foobar\"]}}");
    Document predicate2("{\"a\" :{\"$in\": [\"xyz\", \"abc\"]}}");
    Document predicate3("{\"a\" :{\"$eq\": \"foobar\"}}");
    Document predicate4("{\"a\" :{\"$neq\":\"bla\"}}");
 
    EXPECT_TRUE(doc.matches_predicates(predicate1));
    EXPECT_FALSE(doc.matches_predicates(predicate2));
    EXPECT_TRUE(doc.matches_predicates(predicate3));
    EXPECT_TRUE(doc.matches_predicates(predicate4));
}



TEST(PredicatesTest, int_predicate)
{
    Document doc("{\"a\": 5}");

    Document predicate1("{\"a\" :{\"$lte\":5}}");
    Document predicate2("{\"a\" :{\"$eq\": 3}}");
    Document predicate3("{\"a\" :{\"$gte\": 3}}");
    Document predicate4("{\"a\" :{\"$eq\":5}}");
 
    EXPECT_TRUE(doc.matches_predicates(predicate1));
    EXPECT_FALSE(doc.matches_predicates(predicate2));
    EXPECT_TRUE(doc.matches_predicates(predicate3));
    EXPECT_TRUE(doc.matches_predicates(predicate4));
}

TEST(PredicatesTest, array_predicate)
{
    Document doc("{\"a\":[5,4,{\"c\":3}]}");

    Document predicate1("{\"a.1\":5}");
    Document predicate2("{\"a.0\":5}");
    Document predicate3("{\"a.2.c\":3}");

    EXPECT_FALSE(doc.matches_predicates(predicate1));
    EXPECT_TRUE(doc.matches_predicates(predicate2));
    EXPECT_TRUE(doc.matches_predicates(predicate3));
}

TEST(PredicatesTest, str_equality)
{
    Document doc1("{\"id\": \"xyz\"}");
    Document doc2("{\"id\": \"abc\"}");
    
    Document predicate("{\"id\": \"xyz\"}");

    EXPECT_TRUE(doc1.matches_predicates(predicate));
    EXPECT_FALSE(doc2.matches_predicates(predicate));
}

TEST(PredicatesTest, int_equality)
{
    Document doc1("{\"x\":3, \"id\":42}");
    Document doc2("{\"id\":12, \"y\":5}");
    
    Document predicate("{\"id\": 42}");

    EXPECT_TRUE(doc1.matches_predicates(predicate));
    EXPECT_FALSE(doc2.matches_predicates(predicate));
}

TEST(PredicatesTest, set_predicate)
{
    Document doc1("{\"id\":42}");
    Document doc2("{\"id\":\"whatever\"}");
    Document doc3("{\"id\":1337.0}");

    Document predicate("{\"id\":{\"$in\": [\"whoever\", 1337.0, \"whatever\", \"however\"]}}");

    EXPECT_FALSE(doc1.matches_predicates(predicate));
    EXPECT_TRUE(doc2.matches_predicates(predicate));
    EXPECT_TRUE(doc3.matches_predicates(predicate));
}

TEST(PredicatesTest, wildcard_predicate)
{
    Document doc1("{\"a\":[1,3,4]}");
    Document doc2("{\"a\":[2,5,{\"b\":42}]}");

    Document predicate1("{\"a\":{\"*\":3}}");
    Document predicate2("{\"a.*\":3}");
    Document predicate3("{\"a.*.b\":42}");

    EXPECT_FALSE(doc2.matches_predicates(predicate1));
    EXPECT_TRUE(doc1.matches_predicates(predicate1));
    EXPECT_TRUE(doc1.matches_predicates(predicate2));
    EXPECT_TRUE(doc2.matches_predicates(predicate3));
}
