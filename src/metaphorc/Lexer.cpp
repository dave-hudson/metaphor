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
        indentColumn_(1),
        processingIndent_(false),
        indentOffset_(0),
        inTextBlock_(false),
        seenNonWhitespaceCharacters_(false),
        currentToken_(TokenType::END_OF_FILE, "", "", "", 0, 0) {
    updateEndOfLine();

    if (!std::filesystem::exists(filename)) {
        throw std::runtime_error("File not found: " + filename);
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    input_.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
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

auto Lexer::processIndentation() -> Token {
    if ((indentOffset_ % INDENT_SPACES) == 0) {
        if (indentOffset_ >= INDENT_SPACES) {
            indentOffset_ -= INDENT_SPACES;
            indentColumn_ += INDENT_SPACES;
            return Token(TokenType::INDENT, "[Indent]", line_, filename_, currentLine_, currentColumn_ - currentToken_.value.length() - indentOffset_);
        }

        indentOffset_ += INDENT_SPACES;
        indentColumn_ -= INDENT_SPACES;
        return Token(TokenType::OUTDENT, "[Outdent]", line_, filename_, currentLine_, currentColumn_ - currentToken_.value.length());
    }

    if (indentOffset_ > 0) {
        indentOffset_ = 0;
        return Token(TokenType::BAD_INDENT, "[Bad indent]", line_, filename_, currentLine_, currentColumn_ - currentToken_.value.length() - indentOffset_);
    }

    indentOffset_ = 0;
    return Token(TokenType::BAD_OUTDENT, "[Bad outdent]", line_, filename_, currentLine_, currentColumn_ - currentToken_.value.length());
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

auto Lexer::readKeywordOrText() -> Token {
    int startColumn = currentColumn_;
    size_t startPosition = position_;

    while (position_ < input_.size() && !isspace(input_[position_]) && input_[position_] != '\n') {
        position_++;
        currentColumn_++;
    }

    std::string word = input_.substr(startPosition, position_ - startPosition);

    // If we have a keyword then return that.
    if (keyword_map.find(word) != keyword_map.end()) {
        // Once we've seen a keyword, we're no longer in a text block.
        inTextBlock_ = false;
        seenNonWhitespaceCharacters_ = true;
        return Token(keyword_map.at(word), word, line_, filename_, currentLine_, startColumn);
    }

    // We're dealing with text.  If we're already in a text block then we want to use the same indentation
    // level for all rows of text unless we see outdenting (in which case we've got bad text, but we'll
    // leave that to the parser).
    if (inTextBlock_) {
        if (startColumn > indentColumn_) {
            startColumn = indentColumn_;
        }
    }

    // If we haven't seen anything other than whitespace on this line so far then we can assume we're in a text block.
    // If we have seen characters before they will have been a keyword and this isn't a text block.
    if (!seenNonWhitespaceCharacters_) {
        inTextBlock_ = true;
    }

    seenNonWhitespaceCharacters_ = true;
    position_ = endOfLine_;
    return Token(TokenType::TEXT, line_.substr(startColumn - 1, endOfLine_ - startOfLine_ - (startColumn - 1)), line_, filename_, currentLine_, startColumn);
}

auto Lexer::getNextToken() -> Token {
    // Are we doing any indentation changes?  If yes then ensure we emit the correct
    // stream of tokens before the last actual token we saw.
    if (processingIndent_) {
        if (indentOffset_) {
            return processIndentation();
        }

        processingIndent_ = false;
        return currentToken_;
    }

    // Get the next token.
    while (true) {
        if (position_ >= input_.size()) {
            currentToken_ = Token(TokenType::END_OF_FILE, "", line_, filename_, currentLine_, 1);
            break;
        }

        char ch = input_[position_];

        // If we have a new line then get the next one.
        if (ch == '\n') {
            // If we've not seen any non-whitespace characters and we're in a text block then emit a blank
            // line.  Then pretend we saw characters so next time we process the end of line.
            if (!seenNonWhitespaceCharacters_ && inTextBlock_) {
                seenNonWhitespaceCharacters_ = true;
                currentToken_ = Token(TokenType::TEXT, "", line_, filename_, currentLine_, indentColumn_);
                break;
            }

            processingIndent_ = true;
            consumeNewline();
            updateEndOfLine();
            seenNonWhitespaceCharacters_ = false;
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
        indentOffset_ = currentToken_.column - indentColumn_;
        if (indentOffset_) {
            return processIndentation();
        }

        processingIndent_ = false;
    }

    return currentToken_;
}
