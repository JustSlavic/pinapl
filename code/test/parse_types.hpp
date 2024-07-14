#pragma once


#define RUN_TEST_SUITE__PARSING() \
    printf("        Parsing:\n"); \
    TEST_RUN(ParseUnit); \
    TEST_RUN(ParseInt); \
    TEST_RUN(ParseBool); \
    TEST_RUN(ParseTuple); \
    TEST_RUN(ParseTupleTuples); \
    (void)0


TEST(ParseUnit)
{
    char const source_code[] = "()";
    auto lex = pinapl::lexer::from((void *)source_code, sizeof(source_code));
    pinapl::parser parser = {};
    parser.parse_type(&lex);

    TEST_ASSERT_EQ(parser.ast.size(), 1);
    TEST_ASSERT_EQ(parser.ast[0].kind, pinapl::AST_NODE__TYPE);

    auto t = parser.get_type(&parser.ast[0]);
    TEST_ASSERT_EQ(t.count, 0);
}

TEST(ParseInt)
{
    char const source_code[] = "int";
    auto lex = pinapl::lexer::from((void *)source_code, sizeof(source_code));
    pinapl::parser parser = {};
    parser.parse_type(&lex);

    TEST_ASSERT_EQ(parser.ast.size(), 1);
    TEST_ASSERT_EQ(parser.ast[0].kind, pinapl::AST_NODE__TYPE);

    auto t = parser.get_type(&parser.ast[0]);
    TEST_ASSERT_EQ(t.count, 1);

    uint32 index = t.hash[0] % TYPES_HASH_TABLE_SIZE;
    TEST_ASSERT_EQ(t.name, "int");
    TEST_ASSERT_NEQ(index, 0);
}

TEST(ParseBool)
{
    char const source_code[] = "bool";
    auto lex = pinapl::lexer::from((void *)source_code, sizeof(source_code));
    pinapl::parser parser = {};
    parser.parse_type(&lex);

    TEST_ASSERT_EQ(parser.ast.size(), 1);
    TEST_ASSERT_EQ(parser.ast[0].kind, pinapl::AST_NODE__TYPE);

    auto t = parser.get_type(&parser.ast[0]);
    TEST_ASSERT_EQ(t.count, 1);

    uint32 index = t.hash[0] % TYPES_HASH_TABLE_SIZE;
    TEST_ASSERT_EQ(t.name, "bool");
    TEST_ASSERT_NEQ(index, 0);
}

TEST(ParseTuple)
{
    char const source_code[] = "(bool, int)";
    auto lex = pinapl::lexer::from((void *)source_code, sizeof(source_code));
    pinapl::parser parser = {};
    parser.parse_type(&lex);

    TEST_ASSERT_EQ(parser.ast.size(), 3);
    TEST_ASSERT_EQ(parser.ast[0].kind, pinapl::AST_NODE__TYPE);
    TEST_ASSERT_EQ(parser.ast[1].kind, pinapl::AST_NODE__TYPE);
    TEST_ASSERT_EQ(parser.ast[2].kind, pinapl::AST_NODE__TYPE);

    uint32 index_bool = 0;
    {
        auto t = parser.get_type(&parser.ast[0]);
        TEST_ASSERT_EQ(t.count, 1);
        TEST_ASSERT_EQ(t.name, "bool");

        uint32 index = t.hash[0] % TYPES_HASH_TABLE_SIZE;
        TEST_ASSERT_NEQ(index, 0); // Check bool is not Unit type

        index_bool = index;
    }

    uint32 index_int = 0;
    {
        auto t = parser.get_type(&parser.ast[1]);
        TEST_ASSERT_EQ(t.count, 1);
        TEST_ASSERT_EQ(t.name, "int");

        uint32 index = t.hash[0] % TYPES_HASH_TABLE_SIZE;
        TEST_ASSERT_NEQ(index, 0); // Check int is not Unit type

        index_int = index;
    }

    {
        auto t = parser.get_type(&parser.ast[2]);
        TEST_ASSERT_EQ(t.count, 2);

        uint32 index0 = t.hash[0] % TYPES_HASH_TABLE_SIZE;
        uint32 index1 = t.hash[1] % TYPES_HASH_TABLE_SIZE;

        TEST_ASSERT_NEQ(index0, 0); // Check bool is not Unit type
        TEST_ASSERT_EQ(index0, index_bool); // Check bool is the same bool as above

        TEST_ASSERT_NEQ(index1, 0); // Check int is not Unit type
        TEST_ASSERT_EQ(index1, index_int); // Check int is the same int as above

    }
}

TEST(ParseTupleTuples)
{
    char const source_code[] = "((), ())";
    auto lex = pinapl::lexer::from((void *)source_code, sizeof(source_code));
    pinapl::parser parser = {};
    parser.parse_type(&lex);

    TEST_ASSERT_EQ(parser.ast.size(), 3);
    TEST_ASSERT_EQ(parser.ast[0].kind, pinapl::AST_NODE__TYPE);
    TEST_ASSERT_EQ(parser.ast[1].kind, pinapl::AST_NODE__TYPE);
    TEST_ASSERT_EQ(parser.ast[2].kind, pinapl::AST_NODE__TYPE);

    {
        auto t = parser.get_type(&parser.ast[0]);
        TEST_ASSERT_EQ(t.count, 0);
    }
    {
        auto t = parser.get_type(&parser.ast[1]);
        TEST_ASSERT_EQ(t.count, 0);
    }
}
