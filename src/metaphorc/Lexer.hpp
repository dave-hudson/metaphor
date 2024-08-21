#ifndef __LEXER_HPP
#define __LEXER_HPP

#include <string>
#include <map>

#include "Token.hpp"

class Lexer {
public:
    Lexer(const std::string& filename);

    auto getNextToken() -> Token;

private:
    auto updateEndOfLine() -> void;
    auto processIndentation() -> Token;
    auto consumeNewline() -> void;
    auto consumeWhitespace() -> void;
    auto readKeywordOrText() -> Token;

    std::string filename_;              // File we're lexing
    std::string input_;                 // Content being lexed
    std::string line_;                  // Line curently being lexed
    size_t position_;                   // Offset of the current character being lexed
    size_t startOfLine_;                // Offset of the first character of the current line being lexed
    size_t endOfLine_;                  // Offset of the last character of the current line being lexed
    int currentLine_;                   // Current line number being processed (starting at 1)
    int currentColumn_;                 // Current column number being processed (starting at 1)
    int indentColumn_;                  // Column number for indentation processing
    bool processingIndent_;             // Are we processing indentation at the start of a line?
    int indentOffset_;                  // If we're handling indentation changes, how far do we need to move?
    bool inTextBlock_;                  // Are we processing a text block?
    Token currentToken_;                // Current token we're processing

    const std::map<std::string, TokenType> keyword_map = {
        {"Include:", TokenType::INCLUDE},
        {"Goal:", TokenType::GOAL},
        {"Story:", TokenType::STORY},
        {"As:", TokenType::AS},
        {"I:", TokenType::I},
        {"So:", TokenType::SO},
        {"Require:", TokenType::REQUIRE},
        {"Example:", TokenType::EXAMPLE},
        {"Given:", TokenType::GIVEN},
        {"When:", TokenType::WHEN},
        {"Then:", TokenType::THEN}
    };
};

#endif // __LEXER_HPP