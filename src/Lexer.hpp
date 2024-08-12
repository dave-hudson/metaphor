#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <map>
#include <vector>
#include <ostream>

enum class TokenType {
    FEATURE,
    BACKGROUND,
    SCENARIO,
    GIVEN,
    WHEN,
    THEN,
    AND,
    BUT,
    AS,
    I,
    SO,
    INCLUDE,
    COMMENT,
    NEWLINE,
    WHITESPACE,
    TEXT,
    END_OF_FILE
};

class Token {
public:
    Token(TokenType type, std::string value, int line, int column)
        : type(type), value(std::move(value)), line(line), column(column) {}

    friend std::ostream& operator<<(std::ostream& os, const Token& token) {
        os << "Token(type=" << static_cast<int>(token.type)
            << ", value=\"" << token.value << "\", line=" << token.line
            << ", column=" << token.column << ")";
        return os;
    }

    TokenType type;
    std::string value;
    int line;
    int column;
};

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
