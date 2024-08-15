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
        indentLevel(0) {
}

auto Parser::parseDefine(const Token& defineToken) -> std::unique_ptr<ParseNode> {
    auto defineNode = std::make_unique<ParseNode>(defineToken);
    auto beginNode = getNextToken();
    if (beginNode.type == TokenType::END_OF_FILE) {
        return defineNode;
    }

    if (beginNode.type != TokenType::BEGIN) {
        raiseSyntaxError("Expected indented block");
    }

    indentLevel++;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::END_OF_FILE:
            return defineNode;

        case TokenType::END:
            indentLevel--;
            return defineNode;

        case TokenType::REQUIRE:
            defineNode->addChild(parseRequire(token));
            break;

        case TokenType::TEXT: {
                auto textNode = std::make_unique<ParseNode>(token);
                defineNode->addChild(std::move(textNode));
            }
            break;

        default:
            raiseSyntaxError("Unexpected token in Define block");
        }
    }
}

auto Parser::parseRequire(const Token& requireToken) -> std::unique_ptr<ParseNode> {
    auto requireNode = std::make_unique<ParseNode>(requireToken);

    const auto& token = getNextToken();
    if (token.type == TokenType::TEXT) {
        auto textNode = std::make_unique<ParseNode>(token);
        requireNode->addChild(std::move(textNode));
        return requireNode;
    }

    if (token.type != TokenType::BEGIN) {
        raiseSyntaxError("Expected description or indent");
    }

    indentLevel++;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::END_OF_FILE:
            return requireNode;

        case TokenType::END:
            indentLevel--;
            return requireNode;

        case TokenType::REQUIRE:
            requireNode->addChild(parseRequire(token));
            break;

        case TokenType::EXAMPLE:
            requireNode->addChild(parseExample(token));
            break;

        case TokenType::TEXT: {
                auto textNode = std::make_unique<ParseNode>(token);
                requireNode->addChild(std::move(textNode));
            }
            break;

        default:
            raiseSyntaxError("Unexpected token in Require block");
        }
    }
}

auto Parser::parseExample(const Token& exampleToken) -> std::unique_ptr<ParseNode> {
    auto requireNode = std::make_unique<ParseNode>(exampleToken);

    const auto& token = getNextToken();
    if (token.type == TokenType::TEXT) {
        auto textNode = std::make_unique<ParseNode>(token);
        requireNode->addChild(std::move(textNode));
        return requireNode;
    }

    if (token.type != TokenType::BEGIN) {
        raiseSyntaxError("Expected description or indent");
    }

    indentLevel++;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::END_OF_FILE:
            return requireNode;

        case TokenType::END:
            indentLevel--;
            return requireNode;

        case TokenType::TEXT: {
                auto textNode = std::make_unique<ParseNode>(token);
                requireNode->addChild(std::move(textNode));
            }
            break;

        default:
            raiseSyntaxError("Unexpected token in Example block");
        }
    }
}

auto Parser::parseInclude() -> void {
    const auto& token = getNextToken();
    if (token.type != TokenType::TEXT) {
        raiseSyntaxError("Expected file name");
    }

    std::string filename = token.value;
    loadFile(filename);
}

auto Parser::parse(const std::string& initial_file) -> bool {
    loadFile(initial_file);

    const auto& token = getNextToken();
    if (token.type != TokenType::DEFINE) {
        raiseSyntaxError("Expected Define keyword");
    }

    syntaxTree = parseDefine(token);

    const auto& tokenNext = getNextToken();
    if (tokenNext.type != TokenType::END_OF_FILE) {
        raiseSyntaxError("Already processed Define keyword");
    }

    if (parseErrors.size() > 0) {
        return false;
    }

    syntaxTree->printTree();
    return true;
}

auto Parser::loadFile(const std::string& filename) -> void {
    std::filesystem::path canonicalFilename = std::filesystem::absolute(filename);

    if (processedFiles.find(canonicalFilename) != processedFiles.end()) {
        throw std::runtime_error("Infinite loop detected: '" + canonicalFilename.string() + "' has already been processed.");
    }

    processedFiles.insert(canonicalFilename);

    if (!std::filesystem::exists(canonicalFilename)) {
        throw std::runtime_error("File not found: " + canonicalFilename.string());
    }

    std::ifstream file(canonicalFilename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + canonicalFilename.string());
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    lexers.push_back({std::make_unique<Lexer>(content), canonicalFilename.string(), indentLevel});
    indentLevel = 0;
}

auto Parser::getNextToken() -> Token {
    while (!lexers.empty()) {
        auto& lexerWithFilename = lexers.back();
        currentToken = lexerWithFilename.lexer->getNextToken();

        if (currentToken.type == TokenType::END_OF_FILE) {
            indentLevel = lexers.back().currentIndent;
            lexers.pop_back();
            continue;
        }

        if (currentToken.type == TokenType::INCLUDE) {
            parseInclude();
            continue;
        }

        return currentToken;
    }

    return Token(TokenType::END_OF_FILE, "", 0, 0);
}

auto Parser::raiseSyntaxError(const std::string& message) -> void {
    const auto& token = currentToken;
    std::string currentFile = lexers.empty() ? "Unknown" : lexers.back().filename;
    std::string errorMessage = message + ": Found '" + token.value + "' in file " + currentFile +
                                ", line " + std::to_string(token.line) +
                                ", column " + std::to_string(token.column) + "\n";
    parseErrors.push_back(errorMessage);
}

auto Parser::getSyntaxErrors() -> std::vector<std::string> {
    return parseErrors;
}
