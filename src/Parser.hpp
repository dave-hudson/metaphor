#ifndef __PARSER_HPP
#define __PARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <filesystem>

#include "Lexer.hpp"

class ParseNode {
public:
    TokenType tokenType;
    std::string value;
    int line;
    int column;
    ParseNode* parentNode;
    std::vector<std::unique_ptr<ParseNode>> childNodes;

    ParseNode(const Token& token);
    auto addChild(std::unique_ptr<ParseNode> child) -> void;
    auto printTree(int level = 0) const -> void;
};

class Parser {
public:
    Parser();
    auto parse(const std::string& initial_file) -> bool;
    auto getSyntaxErrors() -> std::vector<std::string>;

private:
    struct LexerWithFilename {
        std::unique_ptr<Lexer> lexer;
        std::string filename;
        int currentIndent;
    };

    auto getNextToken() -> Token;
    auto raiseSyntaxError(const std::string& message) -> void;
    auto parseInclude() -> void;
    auto parseGoal(const Token& defineToken) -> std::unique_ptr<ParseNode>;
    auto parseRequire(const Token& requireToken) -> std::unique_ptr<ParseNode>;
    auto parseExample(const Token& exampleToken) -> std::unique_ptr<ParseNode>;
    auto parseGiven(const Token& givenToken) -> std::unique_ptr<ParseNode>;
    auto parseWhen(const Token& WhenToken) -> std::unique_ptr<ParseNode>;
    auto parseThen(const Token& ThenToken) -> std::unique_ptr<ParseNode>;
    auto parseText(const Token& textToken) -> std::unique_ptr<ParseNode>;
    auto loadFile(const std::string& filename) -> void;

    std::vector<LexerWithFilename> lexers;
    Token currentToken;
    std::set<std::filesystem::path> processedFiles;
    std::unique_ptr<ParseNode> syntaxTree;
    int indentLevel;                    // Indent level withing the current file
    std::vector<std::string> parseErrors;
};

#endif // __PARSER_HPP
