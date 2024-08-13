#include <fstream>

#include "Parser.hpp"

ParseNode::ParseNode(const Token& token) :
        tokenType(token.type),
        value(token.value),
        line(token.line),
        column(token.column),
        parentNode(NULL) {
}

// Method to add a child
auto ParseNode::addChild(std::unique_ptr<ParseNode> child) -> void {
    child->parentNode = this;
    childNodes.push_back(std::move(child));
}

// Method to print the tree for debugging
auto ParseNode::printTree(int level) const -> void {
    std::cout << std::string(level * 2, ' ') << value << std::endl;
    for (const auto& child : childNodes) {
        child->printTree(level + 1);
    }
}

Parser::Parser() :
        currentToken(TokenType::END_OF_FILE, "", 0, 0),
        localIndentLevel(0),
        fileIndentLevel(0) {
}

auto Parser::parseDefine(const Token& defineToken) -> std::unique_ptr<ParseNode> {
    auto defineNode = std::make_unique<ParseNode>(defineToken);

    while (true) {
        const auto& token = getNextSyntaxToken();
        switch (token.type) {
        case TokenType::END_OF_FILE:
            return defineNode;

        case TokenType::INCLUDE:
            parseInclude();
            break;

        case TokenType::REQUIRE:
            defineNode->addChild(parseRequire(token));
            break;

        default:
            std::cout << "token " << static_cast<int>(token.type) << " found\n";
        }
    }

    return defineNode;
}

auto Parser::parseRequire(const Token& requireToken) -> std::unique_ptr<ParseNode> {
    auto requireNode = std::make_unique<ParseNode>(requireToken);
    const auto& token = getNextSyntaxToken();
    if (token.type != TokenType::TEXT) {
        raiseSyntaxError("Expected text description");
    }

    auto textNode = std::make_unique<ParseNode>(token);
    requireNode->addChild(std::move(textNode));
    return requireNode;
}

auto Parser::parseInclude() -> void {
    const auto& token = getNextSyntaxToken();
    if (token.type != TokenType::TEXT) {
        raiseSyntaxError("Expected file name");
    }

    std::string filename = token.value;

    // Preserve the current indent levels for when we return for processing the file, and update
    // so we're tracking the correct starting indent levels for the file.
    auto prevFileIndentLevel = fileIndentLevel;
    auto prevLocalIndentLevel = localIndentLevel;
    fileIndentLevel += localIndentLevel;
    localIndentLevel = 0;
    loadFile(filename);

    // Restore our previous indent levels.
    fileIndentLevel = prevFileIndentLevel;
    localIndentLevel = prevLocalIndentLevel;
}

auto Parser::parse(const std::string& initial_file) -> void {
    loadFile(initial_file);

    const auto& token = getNextSyntaxToken();
    if (token.type != TokenType::DEFINE) {
        raiseSyntaxError("Expected Define keyword");
        return;
    }

    syntaxTree = parseDefine(token);
    syntaxTree->printTree();
}

auto Parser::loadFile(const std::string& filename) -> void {
    std::filesystem::path canonical_filename = std::filesystem::absolute(filename);

    if (processedFiles.find(canonical_filename) != processedFiles.end()) {
        throw std::runtime_error("Infinite loop detected: '" + canonical_filename.string() + "' has already been processed.");
    }

    processedFiles.insert(canonical_filename);

    if (!std::filesystem::exists(canonical_filename)) {
        throw std::runtime_error("File not found: " + canonical_filename.string());
    }

    std::ifstream file(canonical_filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + canonical_filename.string());
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    lexers.push_back({std::make_unique<Lexer>(content), canonical_filename.string()});
}

auto Parser::getNextToken() -> Token {
    while (!lexers.empty()) {
        auto& lexerWithFilename = lexers.back();
        currentToken = lexerWithFilename.lexer->getNextToken();

        if (currentToken.type == TokenType::END_OF_FILE) {
            lexers.pop_back();
            continue;
        }

        return currentToken;
    }

    return Token(TokenType::END_OF_FILE, "", 0, 0);
}

auto Parser::getNextSyntaxToken() -> Token {
    while (true) {
        auto token = getNextToken();
        if (token.type != TokenType::WHITESPACE &&
                token.type != TokenType::COMMENT &&
                token.type != TokenType::NEWLINE) {
            return token;
        }
    }
}

[[noreturn]] auto Parser::raiseSyntaxError(const std::string& message) -> void {
    const auto& token = currentToken;
    std::string current_file = lexers.empty() ? "Unknown" : lexers.back().filename;
    throw std::runtime_error(message + ": Found '" + token.value + "' in file " + current_file +
                                ", line " + std::to_string(token.line) +
                                ", column " + std::to_string(token.column));
}
