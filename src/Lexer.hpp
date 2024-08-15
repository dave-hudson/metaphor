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

    std::string input;                  // Content being lexed
    size_t position;                    // Offset of the current character being lexed
    size_t startOfLine;                 // Offset of the first character of the current line being lexed
    size_t endOfLine;                   // Offset of the last character of the current line being lexed
    int currentLine;                    // Current line number being processed (starting at 1)
    int currentColumn;                  // Current column number being processed (starting at 1)
    int indentColumn;
    bool processingIndent;              // Are we processing indentation at the start of a line?
    int indentOffset; 
    Token currentToken;

    const std::map<std::string, TokenType> keyword_map = {
        {"Include:", TokenType::INCLUDE},
        {"Define:", TokenType::DEFINE},
        {"Require:", TokenType::REQUIRE},
        {"Example:", TokenType::EXAMPLE},
        {"Given:", TokenType::GIVEN},
        {"When:", TokenType::WHEN},
        {"Then:", TokenType::THEN}
    };
};

#endif // __LEXER_HPP
