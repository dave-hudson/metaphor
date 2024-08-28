#ifndef __TOKEN_HPP
#define __TOKEN_HPP

#include <string>
#include <ostream>

enum class TokenType {
    NONE,
    INDENT,
    OUTDENT,
    INJECT,
    EMBED,
    KEYWORD_TEXT,
    TEXT,
    TARGET,
    SCOPE,
    EXAMPLE,
    BAD_INDENT,
    BAD_OUTDENT,
    END_OF_FILE
};

class Token {
public:
    Token(TokenType type, std::string value, std::string input, std::string filename, int line, int column)
            : type(type), value(std::move(value)), input(input), filename(filename), line(line), column(column) {
    }

    friend std::ostream& operator<<(std::ostream& os, const Token& token) {
        os << "Token(type=" << static_cast<int>(token.type)
            << ", value=\"" << token.value << "\", line=" << token.line
            << ", column=" << token.column << ")";
        return os;
    }

    TokenType type;
    std::string value;
    std::string input;
    std::string filename;
    int line;
    int column;
};

#endif // __TOKEN_HPP
