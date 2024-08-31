#ifndef __EMBEDLEXER_HPP
#define __EMBEDLEXER_HPP

#include <string>

#include "Lexer.hpp"

class EmbedLexer : public Lexer {
public:
    EmbedLexer(const std::string& filename);


private:
    auto lexTokens() -> void;
    auto readText() -> Token;
    auto getLanguageFromFilename() -> std::string;
};

#endif // __EMBEDLEXER_HPP
