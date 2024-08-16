#ifndef __TOKEN_HPP
#define __TOKEN_HPP

#include <string>
#include <ostream>

enum class TokenType {
    NONE,
    INDENT,
    OUTDENT,
    INCLUDE,
    TEXT,
    GOAL,
    STORY,
    AS,
    I,
    SO,
    REQUIRE,
    EXAMPLE,
    GIVEN,
    WHEN,
    THEN,
    BAD_INDENT,
    BAD_OUTDENT,
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
    std::string filename;
    std::string input;
};

#endif // __TOKEN_HPP
