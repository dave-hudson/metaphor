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
    auto _getNextToken() -> Token;
    auto readNewline() -> Token;
    auto readKeywordOrText() -> Token;
    auto consumeWhitespace() -> void;
    auto consumeComment() -> void;

    std::string input;
    size_t position;
    int currentLine;
    int currentColumn;
    int indentColumn;
    bool processingIndent;              // Are we processing indentation at this point?
    int indentOffset; 
    Token currentToken;

    const std::map<std::string, TokenType> keyword_map = {
        {"Define:", TokenType::DEFINE},
        {"Require:", TokenType::REQUIRE},
        {"Include:", TokenType::INCLUDE}
    };
};

#endif // __LEXER_HPP
