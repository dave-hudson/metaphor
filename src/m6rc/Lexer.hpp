#ifndef __LEXER_HPP
#define __LEXER_HPP

#include <string>
#include <vector>

#include "Token.hpp"

class Lexer {
public:
    Lexer(const std::string& filename);
    virtual ~Lexer() = default;

    auto getNextToken() -> Token;

protected:
    auto updateEndOfLine() -> void;
    auto consumeNewline() -> void;

    std::string filename_;              // File we're lexing
    std::string input_;                 // Content being lexed
    std::string line_;                  // Line curently being lexed
    std::vector<Token> tokens_;         // All the tokens in the file
    size_t position_;                   // Offset of the current character being lexed
    size_t startOfLine_;                // Offset of the first character of the current line being lexed
    size_t endOfLine_;                  // Offset of the last character of the current line being lexed
    int currentLine_;                   // Current line number being processed (starting at 1)
    int currentColumn_;                 // Current column number being processed (starting at 1)
    bool seenNonWhitespaceCharacters_;  // Have we seen any non-whitespace characters on this line so far?
    size_t nextToken_;                  // Index of the next token to read
};

#endif // __LEXER_HPP
