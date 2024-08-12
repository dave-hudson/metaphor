#ifndef __LEXER_HPP
#define __LEXER_HPP

#include <string>
#include <map>

#include "Token.hpp"

class Lexer {
public:
    Lexer(const std::string& input);

    auto getNextToken() -> Token;

private:
    auto readNewline() -> Token;
    auto readWhitespace() -> Token;
    auto readKeywordOrText() -> Token;
    auto readComment() -> Token;

    std::string input;
    size_t position;
    int currentLine;
    int currentColumn;

    const std::map<std::string, TokenType> keyword_map = {
        {"Feature:", TokenType::FEATURE},
        {"Background:", TokenType::BACKGROUND},
        {"Scenario:", TokenType::SCENARIO},
        {"Given:", TokenType::GIVEN},
        {"When:", TokenType::WHEN},
        {"Then:", TokenType::THEN},
        {"And:", TokenType::AND},
        {"But:", TokenType::BUT},
        {"As:", TokenType::AS},
        {"I:", TokenType::I},
        {"So:", TokenType::SO},
        {"Include:", TokenType::INCLUDE}
    };
};

#endif // __LEXER_HPP
