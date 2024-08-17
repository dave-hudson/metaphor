#include <cctype>
#include <iostream>

#include "Lexer.hpp"

#define INDENT_SPACES 2

Lexer::Lexer(const std::string& input) :
        input_(input),
        position_(0),
        startOfLine_(0),
        endOfLine_(0),
        currentLine_(1),
        currentColumn_(1),
        indentColumn_(1),
        processingIndent_(true),
        indentOffset_(0),
        currentToken_(TokenType::END_OF_FILE, "", 0, 0) {
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
}

auto Lexer::getNextToken() -> Token {
    // Are we doing any indentation changes?  If yes then ensure we emit to correct
    // stream of tokens before the last actual token we saw.
    if (indentOffset_) {
        if (indentOffset_ > 0) {
            indentOffset_ -= INDENT_SPACES;

            // Do we have any more INDENT tokens to emit?  If yes then emit one now.
            if (indentOffset_) {
                if (indentOffset_ < INDENT_SPACES) {
                    indentOffset_ = INDENT_SPACES;
                    return Token(TokenType::BAD_INDENT, "[Bad indent]", currentLine_, currentColumn_ - currentToken_.value.length() - indentOffset_);
                }

                return Token(TokenType::INDENT, "[Indent]", currentLine_, currentColumn_ - currentToken_.value.length() - indentOffset_);
            }
        } else {
            indentOffset_ += INDENT_SPACES;

            // Do we have any more OUTDENT tokens to emit?  If yes then emit one now.
            if (indentOffset_) {
                if (indentOffset_ > -INDENT_SPACES) {
                    indentOffset_ = -INDENT_SPACES;
                    return Token(TokenType::BAD_OUTDENT, "[Bad outdent]", currentLine_, currentColumn_ - currentToken_.value.length());
                }

                return Token(TokenType::OUTDENT, "[Outdent]", currentLine_, currentColumn_ - currentToken_.value.length());
            }
        }

        return currentToken_;
    }

    // Get the next token.
    while (true) {
        if (position_ >= input_.size()) {
            currentToken_ = Token(TokenType::END_OF_FILE, "", currentLine_, 1);
            break;
        }

        char ch = input_[position_];

        // If we have a new line then get the next one.
        if (ch == '\n') {
            processingIndent_ = true;
            consumeNewline();
            updateEndOfLine();
            continue;
        }

        if (isspace(ch)) {
            consumeWhitespace();
            continue;
        }

        // If we have a comment then skip over everything until the end of the current line.
        if (ch == '#') {
            position_ = endOfLine_;
            continue;
        }

        currentToken_ = readKeywordOrText();
        break;
    }

    // If our new token is preceded by indentation then process that first!
    if (processingIndent_) {
        processingIndent_ = false;
        indentOffset_ = currentToken_.column - indentColumn_;
        indentColumn_ = currentToken_.column;
        if (indentOffset_) {
            if (indentOffset_ >= INDENT_SPACES) {
                return Token(TokenType::INDENT, "[Indent]", currentLine_, currentColumn_ - currentToken_.value.length() - indentOffset_);
            }

            if (indentOffset_ <= -INDENT_SPACES) {
                return Token(TokenType::OUTDENT, "[Outdent]", currentLine_, currentColumn_ - currentToken_.value.length());
            }
        }
    }

    return currentToken_;
}

auto Lexer::readKeywordOrText() -> Token {
    int startColumn = currentColumn_;
    size_t startPosition = position_;

    while (position_ < input_.size() && !isspace(input_[position_]) && input_[position_] != '\n') {
        position_++;
        currentColumn_++;
    }

    std::string word = input_.substr(startPosition, position_ - startPosition);

    if (keyword_map.find(word) != keyword_map.end()) {
        return Token(keyword_map.at(word), word, currentLine_, startColumn);
    }

    while (position_ < input_.size() && input_[position_] != '\n' && input_[position_] != '#') {
        position_++;
        currentColumn_++;
    }

    return Token(TokenType::TEXT, input_.substr(startPosition, position_ - startPosition), currentLine_, startColumn);
}

auto Lexer::consumeNewline() -> void {
    position_++;
    currentLine_++;
    currentColumn_ = 1;
}

auto Lexer::consumeWhitespace() -> void {
    while (position_ < input_.size() && isspace(input_[position_]) && input_[position_] != '\n') {
        position_++;
        currentColumn_++;
    }
}

auto Lexer::getCurrentLine() -> std::string {
    return input_.substr(startOfLine_, endOfLine_ - startOfLine_ + 1);
}
