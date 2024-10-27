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
        case TokenType::INCLUDE:
            parseInclude();
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

auto Parser::parseInclude() -> void {
    const auto& token = getNextToken();
    if (token.type != TokenType::KEYWORD_TEXT) {
        raiseSyntaxError(token, "Expected file name for 'Include'");
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

auto Parser::parseAction(const Token& actionToken) -> std::unique_ptr<ASTNode> {
    auto actionNode = std::make_unique<ASTNode>(actionToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::KEYWORD_TEXT) {
        actionNode->addChild(parseKeywordText(initToken));
        const auto& indentToken = getNextToken();
        if (indentToken.type != TokenType::INDENT) {
            raiseSyntaxError(indentToken, "Expected indent for 'Action' block");
        }
    } else if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError(initToken, "Expected description or indent for 'Action' block");
    }

    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError(token, "Text must come first in an 'Action' block");
            }

            actionNode->addChild(parseText(token));
            break;

        case TokenType::CONTEXT:
            actionNode->addChild(parseContext(token));
            seenTokenType = TokenType::CONTEXT;
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            return actionNode;

        default:
            raiseSyntaxError(token, "Unexpected '" + token.value + "' in 'Action' block");
        }
    }
}

auto Parser::parseContext(const Token& contextToken) -> std::unique_ptr<ASTNode> {
    auto contextNode = std::make_unique<ASTNode>(contextToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::KEYWORD_TEXT) {
        contextNode->addChild(parseKeywordText(initToken));
        const auto& indentToken = getNextToken();
        if (indentToken.type != TokenType::INDENT) {
            raiseSyntaxError(indentToken, "Expected indent for 'Context' block");
        }
    } else if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError(initToken, "Expected description or indent for 'Context' block");
    }

    auto seenTokenType = TokenType::NONE;

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            if (seenTokenType != TokenType::NONE) {
                raiseSyntaxError(token, "Text must come first in a 'Context' block");
            }

            contextNode->addChild(parseText(token));
            break;

        case TokenType::CONTEXT:
            contextNode->addChild(parseContext(token));
            seenTokenType = TokenType::CONTEXT;
            break;

        case TokenType::ROLE:
            contextNode->addChild(parseRole(token));
            seenTokenType = TokenType::ROLE;
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            return contextNode;

        default:
            raiseSyntaxError(token, "Unexpected '" + token.value + "' in 'Context' block");
        }
    }
}

auto Parser::parseRole(const Token& roleToken) -> std::unique_ptr<ASTNode> {
    auto roleNode = std::make_unique<ASTNode>(roleToken);

    const auto& initToken = getNextToken();
    if (initToken.type == TokenType::KEYWORD_TEXT) {
        roleNode->addChild(parseKeywordText(initToken));
        const auto& indentToken = getNextToken();
        if (indentToken.type != TokenType::INDENT) {
            raiseSyntaxError(indentToken, "Expected indent for 'Role' block");
        }
    } else if (initToken.type != TokenType::INDENT) {
        raiseSyntaxError(initToken, "Expected description or indent for 'Role' block");
    }

    while (true) {
        const auto& token = getNextToken();
        switch (token.type) {
        case TokenType::TEXT:
            roleNode->addChild(parseText(token));
            break;

        case TokenType::OUTDENT:
        case TokenType::END_OF_FILE:
            return roleNode;

        default:
            raiseSyntaxError(token, "Unexpected '" + token.value + "' in 'Role' block");
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
    if (token.type != TokenType::ACTION) {
        raiseSyntaxError(token, "Expected 'Action' keyword");
    }

    syntaxTree_ = parseAction(token);

    const auto& tokenNext = getNextToken();
    if (tokenNext.type != TokenType::END_OF_FILE) {
        raiseSyntaxError(tokenNext, "Unexpected text after 'Action' block");
    }

    if (parseErrors_.size() > 0) {
        return false;
    }

    return true;
}
