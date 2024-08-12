#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <map>
#include <vector>

#include "Token.hpp"

class Lexer {
public:
    Lexer(const std::string& input);

    Token getNextToken();
    int getCurrentLine() const {
        return current_line;
    }

private:
    Token readNewline();
    Token readWhitespace();
    Token readKeywordOrText();
    Token readComment();

    std::string input;
    size_t position;
    int current_line;
    int current_column;

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

#endif
