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

    void loadFile(const std::string& filename);
    Token nextToken();
    Token nextSyntacticToken();
    void handleInclude();
    void raiseSyntaxError(const std::string& message);

    std::vector<LexerWithFilename> lexers;
    Token current_token;
    std::set<std::filesystem::path> processed_files;
};

#endif // __PARSER_HPP
