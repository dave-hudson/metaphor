#include "Parser.hpp"
#include "EmbedLexer.hpp"
#include "MetaphorLexer.hpp"

Parser::Parser() {
}

auto Parser::getNextToken() -> Token {
    while (!lexers_.empty()) {
        auto& lexer = lexers_.back();
        auto token = lexer->getNextToken();

        switch (token.type) {
        case TokenType::INJECT:
            parseInject();
            break;

        case TokenType::EMBED:
            parseEmbed();
            break;

        case TokenType::END_OF_FILE:
            lexers_.pop_back();
            break;

        default:
            return token;
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

auto Parser::parseInject() -> void {
    const auto& token = getNextToken();
    if (token.type != TokenType::KEYWORD_TEXT) {
        raiseSyntaxError(token, "Expected file name for 'Inject'");
    }

    std::string filename = token.value;
    loadFile(filename);
    lexers_.push_back(std::make_unique<MetaphorLexer>(filename));
}

auto Parser::parseEmbed() -> void {
    const auto& token = getNextToken();
    if (token.type != TokenType::KEYWORD_TEXT) {
        raiseSyntaxError(token, "Expected file name for 'Embed'");
    }

    std::string filename = token.value;
    loadFile(filename);
    lexers_.push_back(std::make_unique<EmbedLexer>(filename));
}

auto Parser::parseKeywordText(const Token& keywordTextToken) -> std::unique_ptr<ASTNode> {
    return std::make_unique<ASTNode>(keywordTextToken);
}

auto Parser::parseText(const Token& textToken) -> std::unique_ptr<ASTNode> {
    return std::make_unique<ASTNode>(textToken);
}

auto Parser::parseTarget(const Token& targetToken) -> std::unique_ptr<ASTNode> {
    auto targetNode = std::make_unique<ASTNode>(targetToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::KEYWORD_TEXT) {
        targetNode->addChild(parseKeywordText(initToken));
        const auto& indentToken = getNextToken();
        if (indentToken.type != TokenType::INDENT) {
            raiseSyntaxError(indentToken, "Expected indent for 'Product' block");
        }
    } else if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError(initToken, "Expected description or indent for 'Product' block");
    }

    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError(token, "Text must come first in a 'Product' block");
            }

            targetNode->addChild(parseText(token));
            break;

        case TokenType::SCOPE:
            targetNode->addChild(parseScope(token));
            seenTokenType = TokenType::SCOPE;
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            return targetNode;

        default:
            raiseSyntaxError(token, "Unexpected '" + token.value + "' in 'Product' block");
        }
    }
}

auto Parser::parseScope(const Token& scopeToken) -> std::unique_ptr<ASTNode> {
    auto scopeNode = std::make_unique<ASTNode>(scopeToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::KEYWORD_TEXT) {
        scopeNode->addChild(parseKeywordText(initToken));
        const auto& indentToken = getNextToken();
        if (indentToken.type != TokenType::INDENT) {
            raiseSyntaxError(indentToken, "Expected indent for 'Scope' block");
        }
    } else if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError(initToken, "Expected description or indent for 'Scope' block");
    }

    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError(token, "Text must come first in a 'Scope' block");
            }

            scopeNode->addChild(parseText(token));
            break;

        case TokenType::SCOPE:
            scopeNode->addChild(parseScope(token));
            seenTokenType = TokenType::SCOPE;
            break;

        case TokenType::EXAMPLE:
            scopeNode->addChild(parseExample(token));
            seenTokenType = TokenType::EXAMPLE;
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            return scopeNode;

        default:
            raiseSyntaxError(token, "Unexpected '" + token.value + "' in 'Scope' block");
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
    if (token.type != TokenType::TARGET) {
        raiseSyntaxError(token, "Expected 'Target' keyword");
    }

    syntaxTree_ = parseTarget(token);

    const auto& tokenNext = getNextToken();
    if (tokenNext.type != TokenType::END_OF_FILE) {
        raiseSyntaxError(tokenNext, "Unexpected text after 'Target' block");
    }

    if (parseErrors_.size() > 0) {
        return false;
    }

    return true;
}
