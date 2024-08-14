#include <cctype>
#include <iostream>

#include "Lexer.hpp"

Lexer::Lexer(const std::string& input)
    : input(input), position(0), currentLine(1), currentColumn(1) {
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

    return Token(TokenType::END_OF_FILE, "", currentLine, currentColumn);
}

auto Lexer::readNewline() -> Token {
    Token token(TokenType::NEWLINE, "", currentLine, currentColumn);
    position++;
    currentLine++;
    currentColumn = 1;
    return token;
}

auto Lexer::readWhitespace() -> Token{
    int startColumn = currentColumn;
    size_t startPosition = position;

    while (position < input.size() && isspace(input[position]) && input[position] != '\n') {
        position++;
        currentColumn++;
    }

    return Token(TokenType::WHITESPACE, input.substr(startPosition, position - startPosition), currentLine, startColumn);
}

auto Lexer::readKeywordOrText() -> Token {
    int start_column = currentColumn;
    size_t start_position = position;

    while (position < input.size() && !isspace(input[position]) && input[position] != '\n') {
        position++;
        currentColumn++;
    }

    std::string word = input.substr(start_position, position - start_position);

    if (keyword_map.find(word) != keyword_map.end()) {
        return Token(keyword_map.at(word), word, currentLine, start_column);
    }

    while (position < input.size() && input[position] != '\n' && input[position] != '#') {
        position++;
        currentColumn++;
    }

    return Token(TokenType::TEXT, input.substr(start_position, position - start_position), currentLine, start_column);
}

auto Lexer::readComment() -> Token {
    int start_column = currentColumn;
    size_t start_position = position;

    while (position < input.size() && input[position] != '\n') {
        position++;
        currentColumn++;
    }

    return Token(TokenType::COMMENT, input.substr(start_position, position - start_position), currentLine, start_column);
}
