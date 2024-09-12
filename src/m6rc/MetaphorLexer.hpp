#ifndef __METAPHORLEXER_HPP
#define __METAPHORLEXER_HPP

#include <string>
#include <map>

#include "Lexer.hpp"

class MetaphorLexer : public Lexer {
public:
    MetaphorLexer(const std::string& filename);


private:
    auto lexTokens() -> void;
    auto processIndentation(size_t column) -> void;
    auto consumeWhitespace() -> void;
    auto readKeywordOrText() -> void;

    int indentColumn_;                  // Column number for indentation processing
    bool processingIndent_;             // Are we processing indentation at the start of a line?
    bool inTextBlock_;                  // Are we processing a text block?

    const std::map<std::string, TokenType> keyword_map = {
        {"Include:", TokenType::INCLUDE},
        {"Embed:", TokenType::EMBED},
        {"Target:", TokenType::TARGET},
        {"Scope:", TokenType::SCOPE},
        {"Example:", TokenType::EXAMPLE}
    };
};

#endif // __METAPHORLEXER_HPP
