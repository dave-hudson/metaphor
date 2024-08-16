#ifndef __LEXER_HPP
#define __LEXER_HPP

#include <string>
#include <map>

#include "Token.hpp"

class Lexer {
public:
    Lexer(const std::string& input);

    auto getNextToken() -> Token;
    auto getCurrentLine() -> std::string;

private:
    auto updateEndOfLine() -> void;
    auto readKeywordOrText() -> Token;
    auto consumeNewline() -> void;
    auto consumeWhitespace() -> void;

    std::string input_;                 // Content being lexed
    size_t position_;                   // Offset of the current character being lexed
    size_t startOfLine_;                // Offset of the first character of the current line being lexed
    size_t endOfLine_;                  // Offset of the last character of the current line being lexed
    int currentLine_;                   // Current line number being processed (starting at 1)
    int currentColumn_;                 // Current column number being processed (starting at 1)
    int indentColumn_;
    bool processingIndent_;             // Are we processing indentation at the start of a line?
    int indentOffset_;
    Token currentToken_;

    const std::map<std::string, TokenType> keyword_map = {
        {"Include:", TokenType::INCLUDE},
        {"Goal:", TokenType::GOAL},
        {"Require:", TokenType::REQUIRE},
        {"Example:", TokenType::EXAMPLE},
        {"Given:", TokenType::GIVEN},
        {"When:", TokenType::WHEN},
        {"Then:", TokenType::THEN}
    };
};

#endif // __LEXER_HPP
