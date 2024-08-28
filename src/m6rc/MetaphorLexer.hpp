#ifndef __METAPHORLEXER_HPP
#define __METAPHORLEXER_HPP

#include <string>
#include <map>

#include "Lexer.hpp"

class MetaphorLexer : public Lexer {
public:
    MetaphorLexer(const std::string& filename);

    auto getNextToken() -> Token;

private:
    auto processIndentation() -> Token;
    auto consumeWhitespace() -> void;
    auto readKeywordOrText() -> Token;

    int indentColumn_;                  // Column number for indentation processing
    bool processingIndent_;             // Are we processing indentation at the start of a line?
    int indentOffset_;                  // If we're handling indentation changes, how far do we need to move?
    bool inTextBlock_;                  // Are we processing a text block?

    const std::map<std::string, TokenType> keyword_map = {
        {"Inject:", TokenType::INJECT},
        {"Embed:", TokenType::EMBED},
        {"Target:", TokenType::TARGET},
        {"Scope:", TokenType::SCOPE},
        {"Example:", TokenType::EXAMPLE}
    };
};

#endif // __METAPHORLEXER_HPP
