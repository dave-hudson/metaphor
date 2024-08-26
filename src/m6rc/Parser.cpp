#include "Parser.hpp"
#include "CodeLexer.hpp"
#include "MetaphorLexer.hpp"

Parser::Parser() :
        currentToken_(TokenType::NONE, "", "", "", 0, 0),
        indentLevel_(0) {
}

auto Parser::getNextToken() -> Token {
    while (!lexers_.empty()) {
        auto& lexer = lexers_.back();
        currentToken_ = lexer->getNextToken();

        switch (currentToken_.type) {
        case TokenType::INDENT:
            indentLevel_++;
            return currentToken_;

        case TokenType::OUTDENT:
            indentLevel_--;
            return currentToken_;

        case TokenType::INCLUDE:
            parseInclude();
            break;

        case TokenType::CODE:
            parseCode();
            break;

        case TokenType::END_OF_FILE:
            lexers_.pop_back();
            break;

        default:
            return currentToken_;
        }
    }

    return Token(TokenType::END_OF_FILE, "", "", "", 0, 0);
}

auto Parser::raiseSyntaxError(const Token& token, const std::string& message) -> void {
    std::string caret = "";
    for (int i = 1; i < token.column; i++) {
        caret += ' ';
    }

    std::string errorMessage = message + ": line " + std::to_string(token.line) +
                                ", column " + std::to_string(token.column) + ", file " + token.filename +
                                "\n" + caret + "|\n" + caret + "v\n" + token.input;
    parseErrors_.push_back(errorMessage);
}

auto Parser::getSyntaxErrors() -> std::vector<std::string> {
    return parseErrors_;
}

auto Parser::loadFile(const std::string& filename) -> void {
    std::filesystem::path canonicalFilename = std::filesystem::absolute(filename);

    if (processedFiles_.find(canonicalFilename) != processedFiles_.end()) {
        throw std::runtime_error("'" + filename + "' has already been read");
    }

    processedFiles_.insert(canonicalFilename);
}

auto Parser::parseInclude() -> void {
    const auto& token = getNextToken();
    if (token.type != TokenType::KEYWORD_TEXT) {
        raiseSyntaxError(token, "Expected file name in 'Include'");
    }

    std::string filename = token.value;
    loadFile(filename);
    lexers_.push_back(std::make_unique<MetaphorLexer>(filename));
}

auto Parser::parseCode() -> void {
    const auto& token = getNextToken();
    if (token.type != TokenType::KEYWORD_TEXT) {
        raiseSyntaxError(token, "Expected file name in 'Code'");
    }

    std::string filename = token.value;
    loadFile(filename);
    lexers_.push_back(std::make_unique<CodeLexer>(filename));
}

auto Parser::parseKeywordText(const Token& keywordTextToken) -> std::unique_ptr<ASTNode> {
    return std::make_unique<ASTNode>(keywordTextToken);
}

auto Parser::parseText(const Token& textToken) -> std::unique_ptr<ASTNode> {
    return std::make_unique<ASTNode>(textToken);
}

auto Parser::parseProduct(const Token& defineToken) -> std::unique_ptr<ASTNode> {
    auto defineNode = std::make_unique<ASTNode>(defineToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::KEYWORD_TEXT) {
        defineNode->addChild(parseKeywordText(initToken));
        const auto& indentToken = getNextToken();
        if (indentToken.type != TokenType::INDENT) {
            raiseSyntaxError(indentToken, "Expected indent for 'Product' block");
        }
    } else if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError(initToken, "Expected description or indent for 'Product' block");
    }

    auto blockIndentLevel = indentLevel_;
    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError(token, "Text must come first in a 'Product' block");
            }

            defineNode->addChild(parseText(token));
            break;

        case TokenType::TRAIT:
            defineNode->addChild(parseTrait(token));
            seenTokenType = TokenType::TRAIT;
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            if (indentLevel_ >= blockIndentLevel) {
                break;
            }

            return defineNode;

        default:
            raiseSyntaxError(token, "Unexpected '" + token.value + "' in 'Product' block");
        }
    }
}

auto Parser::parseTrait(const Token& traitToken) -> std::unique_ptr<ASTNode> {
    auto traitNode = std::make_unique<ASTNode>(traitToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::KEYWORD_TEXT) {
        traitNode->addChild(parseKeywordText(initToken));
        const auto& indentToken = getNextToken();
        if (indentToken.type != TokenType::INDENT) {
            raiseSyntaxError(indentToken, "Expected indent for 'Trait' block");
        }
    } else if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError(initToken, "Expected description or indent for 'Trait' block");
    }

    auto blockIndentLevel = indentLevel_;
    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError(token, "Text must come first in a 'Trait' block");
            }

            traitNode->addChild(parseText(token));
            break;

        case TokenType::TRAIT:
            traitNode->addChild(parseTrait(token));
            seenTokenType = TokenType::TRAIT;
            break;

        case TokenType::EXAMPLE:
            traitNode->addChild(parseExample(token));
            seenTokenType = TokenType::EXAMPLE;
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            if (indentLevel_ >= blockIndentLevel) {
                break;
            }

            return traitNode;

        default:
            raiseSyntaxError(token, "Unexpected '" + token.value + "' in 'Trait' block");
        }
    }
}

auto Parser::parseExample(const Token& exampleToken) -> std::unique_ptr<ASTNode> {
    auto exampleNode = std::make_unique<ASTNode>(exampleToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::KEYWORD_TEXT) {
        exampleNode->addChild(parseKeywordText(initToken));
        const auto& indentToken = getNextToken();
        if (indentToken.type != TokenType::INDENT) {
            raiseSyntaxError(indentToken, "Expected indent for 'Example' block");
        }
    } else if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError(initToken, "Expected description or indent for 'Example' block");
    }

    auto blockIndentLevel = indentLevel_;
    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError(token, "Text must come first in an 'Example' block");
            }

            exampleNode->addChild(parseText(token));
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            if (indentLevel_ >= blockIndentLevel) {
                break;
            }

            return exampleNode;

        default:
            raiseSyntaxError(token, "Unexpected '" + token.value + "' in 'Example' block");
        }
    }
}

auto Parser::getSyntaxTree() -> std::unique_ptr<ASTNode> {
    return std::move(syntaxTree_);
}

auto Parser::parse(const std::string& initial_file) -> bool {
    loadFile(initial_file);
    lexers_.push_back(std::make_unique<MetaphorLexer>(initial_file));

    const auto& token = getNextToken();
    if (token.type != TokenType::PRODUCT) {
        raiseSyntaxError(token, "Expected 'Product' keyword");
    }

    syntaxTree_ = parseProduct(token);

    const auto& tokenNext = getNextToken();
    if (tokenNext.type != TokenType::END_OF_FILE) {
        raiseSyntaxError(tokenNext, "Unexpected text after 'Product' block");
    }

    if (parseErrors_.size() > 0) {
        return false;
    }

    return true;
}
