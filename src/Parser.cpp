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

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        defineNode->addChild(parseText(initToken));
        return defineNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected indent for 'Define' block");
    }

    auto blockIndentLevel = indentLevel;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            defineNode->addChild(parseText(token));
            break;

        case TokenType::REQUIRE:
            defineNode->addChild(parseRequire(token));
            break;

        case TokenType::OUTDENT:
            if (indentLevel >= blockIndentLevel) {
                break;
            }

            return defineNode;

        case TokenType::END_OF_FILE:
            return defineNode;

        default:
            raiseSyntaxError("Unexpected token in 'Define' block");
        }
    }
}

auto Parser::parseRequire(const Token& requireToken) -> std::unique_ptr<ParseNode> {
    auto requireNode = std::make_unique<ParseNode>(requireToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        requireNode->addChild(parseText(initToken));
        return requireNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'Require' block");
    }

    auto blockIndentLevel = indentLevel;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            requireNode->addChild(parseText(token));
            break;

        case TokenType::REQUIRE:
            requireNode->addChild(parseRequire(token));
            break;

        case TokenType::EXAMPLE:
            requireNode->addChild(parseExample(token));
            break;

        case TokenType::OUTDENT:
            if (indentLevel >= blockIndentLevel) {
                break;
            }

            return requireNode;

        case TokenType::END_OF_FILE:
            return requireNode;

        default:
            raiseSyntaxError("Unexpected token in 'Require' block");
        }
    }
}

auto Parser::parseExample(const Token& exampleToken) -> std::unique_ptr<ParseNode> {
    auto requireNode = std::make_unique<ParseNode>(exampleToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        requireNode->addChild(parseText(initToken));
        return requireNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'Example' block");
    }

    auto blockIndentLevel = indentLevel;
    auto seenTokenType = TokenType::EXAMPLE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            requireNode->addChild(parseText(token));
            break;

        case TokenType::GIVEN:
            if (seenTokenType != TokenType::EXAMPLE) {
                raiseSyntaxError("Can only have one 'Given' in an 'Example' block");
            }

            requireNode->addChild(parseGiven(token));
            seenTokenType = TokenType::GIVEN;
            break;

        case TokenType::WHEN:
            if (seenTokenType == TokenType::EXAMPLE) {
                raiseSyntaxError("Require 'Given' before 'When' in an 'Example' block");
            }

            if (seenTokenType == TokenType::WHEN || seenTokenType == TokenType::THEN) {
                raiseSyntaxError("Can only have one 'When' in an 'Example' block");
            }

            requireNode->addChild(parseWhen(token));
            seenTokenType = TokenType::WHEN;
            break;

        case TokenType::THEN:
            if (seenTokenType == TokenType::EXAMPLE || seenTokenType == TokenType::GIVEN) {
                raiseSyntaxError("Require 'Given' and 'When' before 'Then' in an 'Example' block");
            }

            if (seenTokenType == TokenType::THEN) {
                raiseSyntaxError("Can only have one 'Then' in an 'Example' block");
            }

            requireNode->addChild(parseThen(token));
            seenTokenType = TokenType::THEN;
            break;

        case TokenType::OUTDENT:
            if (indentLevel >= blockIndentLevel) {
                break;
            }

            if (seenTokenType == TokenType::WHEN) {
                raiseSyntaxError("No 'Then' in the 'Example' block");
            }

            if (seenTokenType == TokenType::GIVEN) {
                raiseSyntaxError("No 'When' or 'Then' in the 'Example' block");
            }

            return requireNode;

        case TokenType::END_OF_FILE:
            return requireNode;

        default:
            raiseSyntaxError("Unexpected token in 'Example' block");
        }
    }
}

auto Parser::parseGiven(const Token& givenToken) -> std::unique_ptr<ParseNode> {
    auto givenNode = std::make_unique<ParseNode>(givenToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        givenNode->addChild(parseText(initToken));
        return givenNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'Given' block");
    }

    const auto& token = getNextToken();
    if (token.type != TokenType::TEXT) {
        raiseSyntaxError("Expected text for 'Given' block");
    }

    givenNode->addChild(parseText(token));
    return givenNode;
}

auto Parser::parseWhen(const Token& givenToken) -> std::unique_ptr<ParseNode> {
    auto givenNode = std::make_unique<ParseNode>(givenToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        givenNode->addChild(parseText(initToken));
        return givenNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'When' block");
    }

    const auto& token = getNextToken();
    if (token.type != TokenType::TEXT) {
        raiseSyntaxError("Expected text for 'When' block");
    }

    givenNode->addChild(parseText(token));
    return givenNode;
}

auto Parser::parseThen(const Token& givenToken) -> std::unique_ptr<ParseNode> {
    auto givenNode = std::make_unique<ParseNode>(givenToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        givenNode->addChild(parseText(initToken));
        return givenNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'Then' block");
    }

    const auto& token = getNextToken();
    if (token.type != TokenType::TEXT) {
        raiseSyntaxError("Expected text for 'Then' block");
    }

    givenNode->addChild(parseText(token));
    return givenNode;
}

auto Parser::parseText(const Token& textToken) -> std::unique_ptr<ParseNode> {
    return std::make_unique<ParseNode>(textToken);
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
        raiseSyntaxError("Expected 'Define' keyword");
    }

    syntaxTree = parseDefine(token);

    const auto& tokenNext = getNextToken();
    if (tokenNext.type != TokenType::END_OF_FILE) {
        raiseSyntaxError("Unexpected text after 'Define' block");
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

        switch (currentToken.type) {
        case TokenType::END_OF_FILE:
            indentLevel = lexers.back().currentIndent;
            lexers.pop_back();
            break;

        case TokenType::INCLUDE:
            parseInclude();
            break;

        case TokenType::INDENT:
            indentLevel++;
            return currentToken;

        case TokenType::OUTDENT:
            indentLevel--;
            return currentToken;

        default:
            return currentToken;
        }
    }

    return Token(TokenType::END_OF_FILE, "", 0, 0);
}

auto Parser::raiseSyntaxError(const std::string& message) -> void {
    const auto& token = currentToken;
    std::string line = lexers.back().lexer->getCurrentLine();
    std::string caret = "";
    for (int i = 1; i < token.column; i++) {
        caret += ' ';
    }

    std::string currentFile = lexers.empty() ? "Unknown" : lexers.back().filename;
    std::string errorMessage = message + ": Found '" + token.value + "', line " + std::to_string(token.line) +
                                ", column " + std::to_string(token.column) + ", file " + currentFile +
                                "\n" + caret + "|\n" + caret + "v\n" + line;
    parseErrors.push_back(errorMessage);
}

auto Parser::getSyntaxErrors() -> std::vector<std::string> {
    return parseErrors;
}
