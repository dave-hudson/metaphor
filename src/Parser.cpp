#include <fstream>

#include "Parser.hpp"

Parser::Parser() :
        currentToken(TokenType::END_OF_FILE, "", 0, 0),
        localIndentLevel(0),
        fileIndentLevel(0),
        parseState(ParserState::NONE),
        keywordType(TokenType::END_OF_FILE) {
}

auto Parser::parse(const std::string& initial_file) -> void {
    loadFile(initial_file);

    while (true) {
        const auto& token = getNextToken();

        std::cout << "Processing token: " << token << " in file: " << lexers.back().filename << std::endl;

        switch (token.type) {
        case TokenType::NEWLINE:
            parseState = ParserState::NONE;
            break;

        case TokenType::WHITESPACE:
        case TokenType::COMMENT:
            // We do nothing for whitespace or comments.
            break;

        case TokenType::TEXT:
            if (parseState == ParserState::KEYWORD) {
                parseState = ParserState::TEXT;
                handleKeyword();
            }
            break;

        case TokenType::INCLUDE:
        case TokenType::REQUIRE:
            if (parseState == ParserState::NONE) {
                parseState = ParserState::KEYWORD;
                keywordType = token.type;
                break;
            }

            parseState = ParserState::ERROR;
            raiseSyntaxError("Second keyword found on line");
            break;

        case TokenType::END_OF_FILE:
            return;
        }
    }
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

auto Parser::handleInclude() -> void {
    std::string filename = currentToken.value;
    std::cout << "Including file: " << filename << std::endl;

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

auto Parser::handleKeyword() -> void {
    switch (keywordType) {
    case TokenType::INCLUDE:
        handleInclude();
        break;

    default:
        break;
    }
}

auto Parser::raiseSyntaxError(const std::string& message) -> void {
    const auto& token = currentToken;
    std::string current_file = lexers.empty() ? "Unknown" : lexers.back().filename;
    throw std::runtime_error(message + ": Found '" + token.value + "' in file " + current_file +
                                ", line " + std::to_string(token.line) +
                                ", column " + std::to_string(token.column));
}
