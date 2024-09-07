#ifndef __PARSER_HPP
#define __PARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <filesystem>

#include "Lexer.hpp"
#include "ASTNode.hpp"

class Parser {
public:
    Parser();
    auto parse(const std::string& initial_file) -> bool;
    auto getSyntaxTree() -> std::unique_ptr<ASTNode>;
    auto getSyntaxErrors() -> std::vector<std::string>;

private:
    auto getNextToken() -> Token;
    auto raiseSyntaxError(const Token& token, const std::string& message) -> void;
    auto loadFile(const std::string& filename) -> void;
    auto parseInclude() -> void;
    auto parseEmbed() -> void;
    auto parseKeywordText(const Token& textToken) -> std::unique_ptr<ASTNode>;
    auto parseText(const Token& textToken) -> std::unique_ptr<ASTNode>;
    auto parseTarget(const Token& targetToken) -> std::unique_ptr<ASTNode>;
    auto parseScope(const Token& scopeToken) -> std::unique_ptr<ASTNode>;
    auto parseExample(const Token& exampleToken) -> std::unique_ptr<ASTNode>;

    std::vector<std::unique_ptr<Lexer>> lexers_;
                                        // A vector of lexers currently being used for different files.
    std::set<std::filesystem::path> processedFiles_;
                                        // A set of files that have already been included so we can avoid recursion.
    std::unique_ptr<ASTNode> syntaxTree_;
    std::vector<std::string> parseErrors_;
};

#endif // __PARSER_HPP
