#include <cctype>

#include "Lexer.hpp"

Lexer::Lexer(const std::string& input)
    : input(input), position(0), current_line(1), current_column(1) {
}

auto Lexer::getNextToken() -> Token {
    while (position < input.size()) {
        char ch = input[position];

        if (ch == '\n') {
            return readNewline();
        }

        if (isspace(ch)) {
            return readWhitespace();
        }

        if (ch == '#') {
            return readComment();
        }

        return readKeywordOrText();
    }

    return Token(TokenType::END_OF_FILE, "", current_line, current_column);
}

auto Lexer::readNewline() -> Token {
    Token token(TokenType::NEWLINE, "\n", current_line, current_column);
    position++;
    current_line++;
    current_column = 1;
    return token;
}

auto Lexer::readWhitespace() -> Token{
    int start_column = current_column;
    size_t start_position = position;

    while (position < input.size() && isspace(input[position]) && input[position] != '\n') {
        position++;
        current_column++;
    }

    return Token(TokenType::WHITESPACE, input.substr(start_position, position - start_position), current_line, start_column);
}

auto Lexer::readKeywordOrText() -> Token {
    int start_column = current_column;
    size_t start_position = position;

    while (position < input.size() && !isspace(input[position]) && input[position] != '\n') {
        position++;
        current_column++;
    }

    std::string word = input.substr(start_position, position - start_position);

    if (keyword_map.find(word) != keyword_map.end()) {
        return Token(keyword_map.at(word), word, current_line, start_column);
    }

    while (position < input.size() && input[position] != '\n' && input[position] != '#') {
        position++;
        current_column++;
    }

    return Token(TokenType::TEXT, input.substr(start_position, position - start_position), current_line, start_column);
}

auto Lexer::readComment() -> Token {
    int start_column = current_column;
    size_t start_position = position;

    while (position < input.size() && input[position] != '\n') {
        position++;
        current_column++;
    }

    return Token(TokenType::COMMENT, input.substr(start_position, position - start_position), current_line, start_column);
}
