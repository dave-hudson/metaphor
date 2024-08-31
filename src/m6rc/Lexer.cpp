#include <cctype>
#include <iostream>
#include <fstream>

#include "Lexer.hpp"

#define INDENT_SPACES 4

Lexer::Lexer(const std::string& filename) :
        filename_(filename),
        position_(0),
        startOfLine_(0),
        endOfLine_(0),
        currentLine_(1),
        currentColumn_(1),
        seenNonWhitespaceCharacters_(false),
        nextToken_(0) {
    if (!std::filesystem::exists(filename)) {
        throw std::runtime_error("File not found: " + filename);
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    input_.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    updateEndOfLine();
}

auto Lexer::updateEndOfLine() -> void {
    startOfLine_ = position_;

    size_t inputSize = input_.size();
    while (position_ < inputSize) {
        if (input_[position_] == '\n' || !isspace(input_[position_])) {
            break;
        }

        position_++;
        currentColumn_++;
    }

    endOfLine_ = position_;
    while (endOfLine_ < inputSize) {
        if (input_[endOfLine_] == '\n') {
            break;
        }

        endOfLine_++;
    }

    line_ = input_.substr(startOfLine_, endOfLine_ - startOfLine_ + 1);
}

auto Lexer::consumeNewline() -> void {
    position_++;
    currentLine_++;
    currentColumn_ = 1;
}

auto Lexer::getNextToken() -> Token {
    auto token = tokens_[nextToken_++];
    return token;
}