#ifndef __PARSER_HPP
#define __PARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <filesystem>

#include "Lexer.hpp"

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
    auto nextToken() -> Token;
    auto nextSyntacticToken() -> Token;
    auto handleInclude() -> void;
    auto raiseSyntaxError(const std::string& message) -> void;

    std::vector<LexerWithFilename> lexers;
    Token current_token;
    std::set<std::filesystem::path> processed_files;
};

#endif // __PARSER_HPP
