#include <fstream>

#include "Parser.hpp"

Parser::Parser() :
        currentToken_(TokenType::NONE, "", 0, 0),
        indentLevel_(0) {
}

auto Parser::getNextToken() -> Token {
    while (!lexers_.empty()) {
        auto& lexerWithFilename = lexers_.back();
        currentToken_ = lexerWithFilename.lexer_->getNextToken();

        switch (currentToken_.type) {
        case TokenType::END_OF_FILE:
            indentLevel_ = lexers_.back().currentIndent_;
            lexers_.pop_back();
            break;

        case TokenType::INCLUDE:
            parseInclude();
            break;

        case TokenType::INDENT:
            indentLevel_++;
            return currentToken_;

        case TokenType::OUTDENT:
            indentLevel_--;
            return currentToken_;

        default:
            return currentToken_;
        }
    }

    return Token(TokenType::END_OF_FILE, "", 0, 0);
}

auto Parser::raiseSyntaxError(const std::string& message) -> void {
    const auto& token = currentToken_;
    std::string line = lexers_.back().lexer_->getCurrentLine();
    std::string caret = "";
    for (int i = 1; i < token.column; i++) {
        caret += ' ';
    }

    std::string currentFile = lexers_.empty() ? "Unknown" : lexers_.back().filename_;
    std::string errorMessage = message + ": line " + std::to_string(token.line) +
                                ", column " + std::to_string(token.column) + ", file " + currentFile +
                                "\n" + caret + "|\n" + caret + "v\n" + line;
    parseErrors_.push_back(errorMessage);
}

auto Parser::getSyntaxErrors() -> std::vector<std::string> {
    return parseErrors_;
}

auto Parser::loadFile(const std::string& filename) -> void {
    std::filesystem::path canonicalFilename = std::filesystem::absolute(filename);

    if (processedFiles_.find(canonicalFilename) != processedFiles_.end()) {
        throw std::runtime_error("'" + filename + "' has already been read via 'Include'");
    }

    processedFiles_.insert(canonicalFilename);

    if (!std::filesystem::exists(canonicalFilename)) {
        throw std::runtime_error("File not found: " + filename);
    }

    std::ifstream file(canonicalFilename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    lexers_.push_back({std::make_unique<Lexer>(content), filename, indentLevel_});
    indentLevel_ = 0;
}

auto Parser::parseInclude() -> void {
    const auto& token = getNextToken();
    if (token.type != TokenType::TEXT) {
        raiseSyntaxError("Expected file name in 'Include'");
    }

    std::string filename = token.value;
    loadFile(filename);
}

auto Parser::parseGoal(const Token& defineToken) -> std::unique_ptr<ASTNode> {
    auto defineNode = std::make_unique<ASTNode>(defineToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        defineNode->addChild(parseText(initToken));
        return defineNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected indent for 'Goal' block");
    }

    auto blockIndentLevel = indentLevel_;
    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError("Text must come first in a 'Goal' block");
            }

            defineNode->addChild(parseText(token));
            break;

        case TokenType::STORY:
            defineNode->addChild(parseStory(token));
            seenTokenType = TokenType::STORY;
            break;

        case TokenType::REQUIRE:
            defineNode->addChild(parseRequire(token));
            seenTokenType = TokenType::REQUIRE;
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            if (indentLevel_ >= blockIndentLevel) {
                break;
            }

            return defineNode;

        default:
            raiseSyntaxError("Unexpected '" + token.value + "' in 'Goal' block");
        }
    }
}

auto Parser::parseStory(const Token& storyToken) -> std::unique_ptr<ASTNode> {
    auto storyNode = std::make_unique<ASTNode>(storyToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        storyNode->addChild(parseText(initToken));
        return storyNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'Story' block");
    }

    auto blockIndentLevel = indentLevel_;
    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError("Text must come first in a 'Story' block");
            }

            storyNode->addChild(parseText(token));
            break;

        case TokenType::AS:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError("Can only have one 'As' in a 'Story' block");
            }

            storyNode->addChild(parseGiven(token));
            seenTokenType = TokenType::AS;
            break;

        case TokenType::I:
            if (seenTokenType == TokenType::NONE) {
                raiseSyntaxError("Require 'As' before 'I' in a 'Story' block");
            }

            if (seenTokenType == TokenType::I || seenTokenType == TokenType::SO) {
                raiseSyntaxError("Can only have one 'I' in a 'Story' block");
            }

            storyNode->addChild(parseWhen(token));
            seenTokenType = TokenType::I;
            break;

        case TokenType::SO:
            if (seenTokenType == TokenType::NONE || seenTokenType == TokenType::AS) {
                raiseSyntaxError("Require 'As' and 'I' before 'So' in a 'Story' block");
            }

            if (seenTokenType == TokenType::SO) {
                raiseSyntaxError("Can only have one 'So' in a 'Story' block");
            }

            storyNode->addChild(parseThen(token));
            seenTokenType = TokenType::THEN;
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            if (indentLevel_ >= blockIndentLevel) {
                break;
            }

            if (seenTokenType == TokenType::I) {
                raiseSyntaxError("No 'So' in the 'Story' block");
            }

            if (seenTokenType == TokenType::AS) {
                raiseSyntaxError("No 'I' or 'So' in the 'Story' block");
            }

            return storyNode;

        default:
            raiseSyntaxError("Unexpected '" + token.value + "' in 'Story' block");
        }
    }
}

auto Parser::parseAs(const Token& asToken) -> std::unique_ptr<ASTNode> {
    auto givenNode = std::make_unique<ASTNode>(asToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        givenNode->addChild(parseText(initToken));
        return givenNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'As' block");
    }

    const auto& token = getNextToken();
    if (token.type != TokenType::TEXT) {
        raiseSyntaxError("Expected text for 'As' block");
    }

    givenNode->addChild(parseText(token));
    return givenNode;
}

auto Parser::parseI(const Token& iToken) -> std::unique_ptr<ASTNode> {
    auto givenNode = std::make_unique<ASTNode>(iToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        givenNode->addChild(parseText(initToken));
        return givenNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'I' block");
    }

    const auto& token = getNextToken();
    if (token.type != TokenType::TEXT) {
        raiseSyntaxError("Expected text for 'I' block");
    }

    givenNode->addChild(parseText(token));
    return givenNode;
}

auto Parser::parseSo(const Token& soToken) -> std::unique_ptr<ASTNode> {
    auto givenNode = std::make_unique<ASTNode>(soToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        givenNode->addChild(parseText(initToken));
        return givenNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'So' block");
    }

    const auto& token = getNextToken();
    if (token.type != TokenType::TEXT) {
        raiseSyntaxError("Expected text for 'So' block");
    }

    givenNode->addChild(parseText(token));
    return givenNode;
}

auto Parser::parseRequire(const Token& requireToken) -> std::unique_ptr<ASTNode> {
    auto requireNode = std::make_unique<ASTNode>(requireToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        requireNode->addChild(parseText(initToken));
        return requireNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'Require' block");
    }

    auto blockIndentLevel = indentLevel_;
    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError("Text must come first in a 'Require' block");
            }

            requireNode->addChild(parseText(token));
            break;

        case TokenType::REQUIRE:
            requireNode->addChild(parseRequire(token));
            seenTokenType = TokenType::REQUIRE;
            break;

        case TokenType::EXAMPLE:
            requireNode->addChild(parseExample(token));
            seenTokenType = TokenType::EXAMPLE;
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            if (indentLevel_ >= blockIndentLevel) {
                break;
            }

            return requireNode;

        default:
            raiseSyntaxError("Unexpected '" + token.value + "' in 'Require' block");
        }
    }
}

auto Parser::parseExample(const Token& exampleToken) -> std::unique_ptr<ASTNode> {
    auto exampleNode = std::make_unique<ASTNode>(exampleToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::TEXT) {
        exampleNode->addChild(parseText(initToken));
        return exampleNode;
    }

    if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError("Expected description or indent for 'Example' block");
    }

    auto blockIndentLevel = indentLevel_;
    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError("Text must come first in an 'Example' block");
            }

            exampleNode->addChild(parseText(token));
            break;

        case TokenType::GIVEN:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError("Can only have one 'Given' in an 'Example' block");
            }

            exampleNode->addChild(parseGiven(token));
            seenTokenType = TokenType::GIVEN;
            break;

        case TokenType::WHEN:
            if (seenTokenType == TokenType::NONE) {
                raiseSyntaxError("Require 'Given' before 'When' in an 'Example' block");
            }

            if (seenTokenType == TokenType::WHEN || seenTokenType == TokenType::THEN) {
                raiseSyntaxError("Can only have one 'When' in an 'Example' block");
            }

            exampleNode->addChild(parseWhen(token));
            seenTokenType = TokenType::WHEN;
            break;

        case TokenType::THEN:
            if (seenTokenType == TokenType::NONE || seenTokenType == TokenType::GIVEN) {
                raiseSyntaxError("Require 'Given' and 'When' before 'Then' in an 'Example' block");
            }

            if (seenTokenType == TokenType::THEN) {
                raiseSyntaxError("Can only have one 'Then' in an 'Example' block");
            }

            exampleNode->addChild(parseThen(token));
            seenTokenType = TokenType::THEN;
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            if (indentLevel_ >= blockIndentLevel) {
                break;
            }

            if (seenTokenType == TokenType::WHEN) {
                raiseSyntaxError("No 'Then' in the 'Example' block");
            }

            if (seenTokenType == TokenType::GIVEN) {
                raiseSyntaxError("No 'When' or 'Then' in the 'Example' block");
            }

            return exampleNode;

        default:
            raiseSyntaxError("Unexpected '" + token.value + "' in 'Example' block");
        }
    }
}

auto Parser::parseGiven(const Token& givenToken) -> std::unique_ptr<ASTNode> {
    auto givenNode = std::make_unique<ASTNode>(givenToken);

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

auto Parser::parseWhen(const Token& whenToken) -> std::unique_ptr<ASTNode> {
    auto givenNode = std::make_unique<ASTNode>(whenToken);

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

auto Parser::parseThen(const Token& thenToken) -> std::unique_ptr<ASTNode> {
    auto givenNode = std::make_unique<ASTNode>(thenToken);

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

auto Parser::parseText(const Token& textToken) -> std::unique_ptr<ASTNode> {
    return std::make_unique<ASTNode>(textToken);
}

auto Parser::parse(const std::string& initial_file) -> bool {
    loadFile(initial_file);

    const auto& token = getNextToken();
    if (token.type != TokenType::GOAL) {
        raiseSyntaxError("Expected 'Goal' keyword");
    }

    syntaxTree_ = parseGoal(token);

    const auto& tokenNext = getNextToken();
    if (tokenNext.type != TokenType::END_OF_FILE) {
        raiseSyntaxError("Unexpected text after 'Goal' block");
    }

    if (parseErrors_.size() > 0) {
        return false;
    }

    syntaxTree_->printTree();
    return true;
}
