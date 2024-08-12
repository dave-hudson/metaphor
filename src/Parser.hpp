#ifndef __PARSER_HPP
#define __PARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <filesystem>

#include "Lexer.hpp"

enum class ParserState {
    NONE,
    KEYWORD,
    TEXT,
    ERROR
};

class Parser {
public:
    Parser();
    void parse(const std::string& initial_file);

private:
    struct LexerWithFilename {
        std::unique_ptr<Lexer> lexer;
        std::string filename;
    };

    auto loadFile(const std::string& filename) -> void;
    auto getNextToken() -> Token;
    auto handleInclude() -> void;
    auto handleKeyword() -> void;
    auto raiseSyntaxError(const std::string& message) -> void;

    std::vector<LexerWithFilename> lexers;
    Token currentToken;
    std::set<std::filesystem::path> processedFiles;
    int localIndentLevel;               // Indent level withing the current file
    int fileIndentLevel;                // Base indent level for the current file
    ParserState parseState;             // The current parser state
    TokenType keywordType;              // The current keyword being processed
};

#endif // __PARSER_HPP
