#include "Parser.hpp"

Parser::Parser() : current_token(TokenType::END_OF_FILE, "", 0, 0) {
    // Constructor implementation
}

void Parser::parse(const std::string& initial_file) {
    loadFile(initial_file);

    while (nextSyntacticToken().type != TokenType::END_OF_FILE) {
        const auto& token = current_token;

        std::cout << "Processing token: " << token << " in file: " << lexers.back().filename << std::endl;

        if (token.type == TokenType::INCLUDE) {
            handleInclude();
        } else {
            // Handle other tokens as necessary
        }
    }
}

void Parser::loadFile(const std::string& filename) {
    std::filesystem::path canonical_filename = std::filesystem::absolute(filename);

    if (processed_files.find(canonical_filename) != processed_files.end()) {
        throw std::runtime_error("Infinite loop detected: '" + canonical_filename.string() + "' has already been processed.");
    }

    processed_files.insert(canonical_filename);

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

Token Parser::nextToken() {
    while (!lexers.empty()) {
        auto& lexerWithFilename = lexers.back();
        current_token = lexerWithFilename.lexer->getNextToken();

        if (current_token.type == TokenType::END_OF_FILE) {
            lexers.pop_back();
        } else {
            return current_token;
        }
    }

    return Token(TokenType::END_OF_FILE, "", 0, 0);
}

Token Parser::nextSyntacticToken() {
    Token token = nextToken();

    while (token.type == TokenType::COMMENT || token.type == TokenType::WHITESPACE) {
        token = nextToken();
    }

    return token;
}

void Parser::handleInclude() {
    if (nextSyntacticToken().type == TokenType::TEXT) {
        std::string filename = current_token.value;
        std::cout << "Including file: " << filename << std::endl;
        loadFile(filename);
    } else {
        raiseSyntaxError("Expected a filename (TEXT token) after Include:");
    }
}

void Parser::raiseSyntaxError(const std::string& message) {
    const auto& token = current_token;
    std::string current_file = lexers.empty() ? "Unknown" : lexers.back().filename;
    throw std::runtime_error(message + " Found '" + token.value + "' in file " + current_file +
                                ", line " + std::to_string(token.line) +
                                ", column " + std::to_string(token.column));
}
