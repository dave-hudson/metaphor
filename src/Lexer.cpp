#include <cctype>
#include <iostream>

#include "Lexer.hpp"

#define INDENT_SPACES 2

Lexer::Lexer(const std::string& input) :
        input(input),
        position(0),
        currentLine(1),
        currentColumn(1),
        indentColumn(1),
        processingIndent(true),
        indentOffset(0),
        currentToken(TokenType::END_OF_FILE, "", 0, 0) {
}

auto Lexer::getNextToken() -> Token {
    // Are we doing any indentation changes?  If yes then ensure we emit to correct
    // stream of tokens before the last actual token we saw.
    if (indentOffset) {
        if (indentOffset > 0) {
            indentOffset -= INDENT_SPACES;

            // Do we have any more BEGIN tokens to emit?  If yes then emit one now.
            if (indentOffset) {
                return Token(TokenType::BEGIN, "", currentLine, currentColumn - currentToken.value.length());
            }
        } else {
            indentOffset += INDENT_SPACES;

            // Do we have any more END tokens to emit?  If yes then emit one now.
            if (indentOffset) {
                return Token(TokenType::END, "", currentLine, currentColumn - currentToken.value.length());
            }
        }

        return currentToken;
    }

    // Get the next token.
    while (true) {
        if (position >= input.size()) {
            return Token(TokenType::END_OF_FILE, "", currentLine, currentColumn);
        }

        char ch = input[position];

        if (ch == '\n') {
            processingIndent = true;
            consumeNewline();
            continue;
        }

        if (isspace(ch)) {
            consumeWhitespace();
            continue;
        }

        if (ch == '#') {
            consumeComment();
            continue;
        }

        currentToken = readKeywordOrText();
        break;
    }

    // If our new token is preceded by indentation then process that first!
    if (processingIndent) {
        processingIndent = false;
        indentOffset = currentToken.column - indentColumn;
        indentColumn = currentToken.column;
        if (indentOffset) {
            if (indentOffset % INDENT_SPACES) {
                currentToken.type = TokenType::BAD_INDENT;
                return currentToken;
            }

            if (indentOffset > 0) {
                return Token(TokenType::BEGIN, "", currentLine, currentColumn - currentToken.value.length());
            }

            return Token(TokenType::END, "", currentLine, currentColumn - currentToken.value.length());
        }
    }

    return currentToken;
}

auto Lexer::readKeywordOrText() -> Token {
    int startColumn = currentColumn;
    size_t startPosition = position;

    while (position < input.size() && !isspace(input[position]) && input[position] != '\n') {
        position++;
        currentColumn++;
    }

    std::string word = input.substr(startPosition, position - startPosition);

    if (keyword_map.find(word) != keyword_map.end()) {
        return Token(keyword_map.at(word), word, currentLine, startColumn);
    }

    while (position < input.size() && input[position] != '\n' && input[position] != '#') {
        position++;
        currentColumn++;
    }

    return Token(TokenType::TEXT, input.substr(startPosition, position - startPosition), currentLine, startColumn);
}

auto Lexer::consumeNewline() -> void {
    position++;
    currentLine++;
    currentColumn = 1;
}

auto Lexer::consumeWhitespace() -> void {
    while (position < input.size() && isspace(input[position]) && input[position] != '\n') {
        position++;
        currentColumn++;
    }
}

auto Lexer::consumeComment() -> void {
    while (position < input.size() && input[position] != '\n') {
        position++;
        currentColumn++;
    }
}
