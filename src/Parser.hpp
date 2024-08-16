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
    auto getSyntaxErrors() -> std::vector<std::string>;

private:
    struct LexerWithFilename {
        std::unique_ptr<Lexer> lexer_;  // A lexer
        std::string filename_;          // The filename associated with the lexer
        int currentIndent_;             // The current indentation level when this lexer was invoked
    };

    auto getNextToken() -> Token;
    auto raiseSyntaxError(const std::string& message) -> void;
    auto loadFile(const std::string& filename) -> void;
    auto parseInclude() -> void;
    auto parseGoal(const Token& defineToken) -> std::unique_ptr<ASTNode>;
    auto parseRequire(const Token& requireToken) -> std::unique_ptr<ASTNode>;
    auto parseExample(const Token& exampleToken) -> std::unique_ptr<ASTNode>;
    auto parseGiven(const Token& givenToken) -> std::unique_ptr<ASTNode>;
    auto parseWhen(const Token& WhenToken) -> std::unique_ptr<ASTNode>;
    auto parseThen(const Token& ThenToken) -> std::unique_ptr<ASTNode>;
    auto parseText(const Token& textToken) -> std::unique_ptr<ASTNode>;

    std::vector<LexerWithFilename> lexers_;
                                        // A vector of lexers currently being used for different files.
    Token currentToken_;
    std::set<std::filesystem::path> processedFiles_;
                                        // A set of files that have already been included so we can avoid recursion.
    std::unique_ptr<ASTNode> syntaxTree_;
    int indentLevel_;                   // Indent level withing the current file
    std::vector<std::string> parseErrors_;
};

#endif // __PARSER_HPP
