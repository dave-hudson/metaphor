#ifndef __CODELEXER_HPP
#define __CODELEXER_HPP

#include <string>

#include "Lexer.hpp"

class CodeLexer : public Lexer {
public:
    CodeLexer(const std::string& filename);

    auto getNextToken() -> Token;

private:
    auto updateEndOfLine() -> void;
    auto readText() -> Token;
    auto getLanguageFromFilename() -> std::string;

    bool emitFilename_;                 // Should we emit a file name?
    bool emitFormatDelimeter_;          // Should we emit a format delimeter and not file text?
    bool emitEndOfFile_;                // Should we emit an end of file token?
};

#endif // __CODELEXER_HPP
