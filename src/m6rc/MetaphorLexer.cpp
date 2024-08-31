#include <cctype>
#include <iostream>
#include <fstream>

#include "MetaphorLexer.hpp"

#define INDENT_SPACES 4

MetaphorLexer::MetaphorLexer(const std::string& filename) :
        Lexer(filename),
        indentColumn_(1),
        processingIndent_(false),
        indentOffset_(0),
        inTextBlock_(false) {
    lexTokens();
}

auto MetaphorLexer::processIndentation(size_t column) -> void {
    while (indentOffset_) {
        if ((indentOffset_ % INDENT_SPACES) == 0) {
            if (indentOffset_ >= INDENT_SPACES) {
                indentOffset_ -= INDENT_SPACES;
                indentColumn_ += INDENT_SPACES;
                tokens_.push_back(Token(TokenType::INDENT, "[Indent]", line_, filename_, currentLine_, column));
                continue;
            }

            indentOffset_ += INDENT_SPACES;
            indentColumn_ -= INDENT_SPACES;
            tokens_.push_back(Token(TokenType::OUTDENT, "[Outdent]", line_, filename_, currentLine_, column));
            continue;
        }

        if (indentOffset_ > 0) {
            indentOffset_ = 0;
            tokens_.push_back(Token(TokenType::BAD_INDENT, "[Bad indent]", line_, filename_, currentLine_, column));
        }

        indentOffset_ = 0;
        tokens_.push_back(Token(TokenType::BAD_OUTDENT, "[Bad outdent]", line_, filename_, currentLine_, column));
    }
}

auto MetaphorLexer::consumeWhitespace() -> void {
    while (position_ < input_.size() && isspace(input_[position_]) && input_[position_] != '\n') {
        position_++;
        currentColumn_++;
    }
}

auto MetaphorLexer::readKeywordOrText() -> Token {
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
        return Token(keyword_map.at(word), word, line_, filename_, currentLine_, startColumn);
    }

    // Have we already seen a keyword?  If yes then this is keyword text
    if (seenNonWhitespaceCharacters_) {
        position_ = endOfLine_;
        return Token(TokenType::KEYWORD_TEXT, line_.substr(startColumn - 1, endOfLine_ - startOfLine_ - (startColumn - 1)), line_, filename_, currentLine_, startColumn);
    }

    // We're dealing with text.  If we're already in a text block then we want to use the same indentation
    // level for all rows of text unless we see outdenting (in which case we've got bad text, but we'll
    // leave that to the parser).
    if (inTextBlock_) {
        if (startColumn > indentColumn_) {
            startColumn = indentColumn_;
        }
    }

    inTextBlock_ = true;
    position_ = endOfLine_;
    return Token(TokenType::TEXT, line_.substr(startColumn - 1, endOfLine_ - startOfLine_ - (startColumn - 1)), line_, filename_, currentLine_, startColumn);
}

auto MetaphorLexer::lexTokens() -> void {
    // Get the next token.
    while (position_ < input_.size()) {
        char ch = input_[position_];

        // If we have a new line then get the next one.
        if (ch == '\n') {
            // If we've not seen any non-whitespace characters and we're in a text block then emit a blank
            // line.  Then pretend we saw characters so next time we process the end of line.
            if (!seenNonWhitespaceCharacters_ && inTextBlock_) {
                tokens_.push_back(Token(TokenType::TEXT, "", line_, filename_, currentLine_, indentColumn_));
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

        auto token = readKeywordOrText();
        seenNonWhitespaceCharacters_ = true;

        if (processingIndent_) {
            indentOffset_ = token.column - indentColumn_;
            processIndentation(token.column);
            processingIndent_ = false;
        }

        tokens_.push_back(token);
    }

    tokens_.push_back(Token(TokenType::END_OF_FILE, "", line_, filename_, currentLine_, 1));
}
