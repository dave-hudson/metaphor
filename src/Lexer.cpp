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

auto Lexer::_getNextToken() -> Token {
    while (position < input.size()) {
        char ch = input[position];

        if (ch == '\n') {
            return readNewline();
        }

        if (isspace(ch)) {
            consumeWhitespace();
            continue;
        }

        if (ch == '#') {
            consumeComment();
            continue;
        }
        return readKeywordOrText();
    }

    return Token(TokenType::END_OF_FILE, "", currentLine, currentColumn);
}

auto Lexer::getNextToken() -> Token {
    if (indentOffset) {
        if (indentOffset > 0) {
            indentOffset -= INDENT_SPACES;

            // Do we have any more BEGIN tokens to emit?  If yes then emit one now.
            if (indentOffset) {
                return Token(TokenType::BEGIN, "", 0, 0);
            }
        } else {
            indentOffset += INDENT_SPACES;

            // Do we have any more END tokens to emit?  If yes then emit one now.
            if (indentOffset) {
                return Token(TokenType::END, "", 0, 0);
            }
        }

        // The last END is actually the token that triggered the END processing.
        return currentToken;
    }

    // Get the next token.  Note we do some special processing if we see a newline, but we
    // don't want to return that back to our caller.
    while (true) {
        currentToken = _getNextToken();

        if (currentToken.type == TokenType::NEWLINE) {
            processingIndent = true;
            continue;
        }

        if (currentToken.type == TokenType::END_OF_FILE) {
            return currentToken;
        }

        break;
    }

    if (processingIndent) {
        processingIndent = false;
        indentOffset = currentToken.column - indentColumn;
        indentColumn = currentToken.column;
        if (indentOffset) {
            if (indentOffset > 0) {
                return Token(TokenType::BEGIN, "", 0, 0);
            }

            return Token(TokenType::END, "", 0, 0);
        }
    }

    return currentToken;
}

auto Lexer::readNewline() -> Token {
    Token token(TokenType::NEWLINE, "", currentLine, currentColumn);
    position++;
    currentLine++;
    currentColumn = 1;
    return token;
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
