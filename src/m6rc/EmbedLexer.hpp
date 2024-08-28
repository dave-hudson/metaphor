#ifndef __EMBEDLEXER_HPP
#define __EMBEDLEXER_HPP

#include <string>

#include "Lexer.hpp"

class EmbedLexer : public Lexer {
public:
    EmbedLexer(const std::string& filename);

    auto getNextToken() -> Token;

private:
    auto updateEndOfLine() -> void;
    auto readText() -> Token;
    auto getLanguageFromFilename() -> std::string;

    bool emitFilename_;                 // Should we emit a file name?
    bool emitFormatDelimeter_;          // Should we emit a format delimeter and not file text?
    bool emitEndOfFile_;                // Should we emit an end of file token?
};

#endif // __EMBEDLEXER_HPP
