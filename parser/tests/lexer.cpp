#include <string>

#include <gtest/gtest.h>

#include "parser/metalexer.h"

TEST(Lexer, identifierVsQname) {
    const char *input = "testIdentifier test.qname";
    meta::Lexer lexer;
    lexer.start(input);
    lexer.next();
    ASSERT_EQ(std::string(lexer.currentToken().start, lexer.currentToken().end - lexer.currentToken().start), std::string("testIdentifier"));
    lexer.next();
    ASSERT_EQ(std::string(lexer.currentToken().start, lexer.currentToken().end - lexer.currentToken().start), std::string("test.qname"));
}

TEST(Lexer, separated) {
    const char *input = "testIdentifier1;testIdentifier2,test.qname";
    meta::Lexer lexer;
    lexer.start(input);
    lexer.next();
    ASSERT_EQ(std::string(lexer.currentToken().start, lexer.currentToken().end - lexer.currentToken().start), std::string("testIdentifier1"));
    lexer.next();
    ASSERT_EQ(std::string(lexer.currentToken().start, lexer.currentToken().end - lexer.currentToken().start), std::string(";"));
    lexer.next();
    ASSERT_EQ(std::string(lexer.currentToken().start, lexer.currentToken().end - lexer.currentToken().start), std::string("testIdentifier2"));
    lexer.next();
    ASSERT_EQ(std::string(lexer.currentToken().start, lexer.currentToken().end - lexer.currentToken().start), std::string(","));
    lexer.next();
    ASSERT_EQ(std::string(lexer.currentToken().start, lexer.currentToken().end - lexer.currentToken().start), std::string("test.qname"));
}