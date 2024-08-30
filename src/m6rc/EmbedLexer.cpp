#include <iostream>
#include <fstream>
#include <unordered_map>

#include "EmbedLexer.hpp"

EmbedLexer::EmbedLexer(const std::string& filename) :
        Lexer(filename),
        emitFilename_(true),
        emitFormatDelimeter_(true),
        emitEndOfFile_(false) {
}

auto EmbedLexer::updateEndOfLine() -> void {
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

auto EmbedLexer::readText() -> Token {
    position_ = endOfLine_;
    return Token(TokenType::TEXT, line_.substr(0, endOfLine_ - startOfLine_), line_, filename_, currentLine_, 1);
}

auto EmbedLexer::getLanguageFromFilename() -> std::string {
    static const std::unordered_map<std::string, std::string> extensionToLanguage = {
        {".bash", "bash"},
        {".c", "c"},
        {".clj", "clojure"},
        {".cpp", "cpp"},
        {".cs", "csharp"},
        {".css", "css"},
        {".dart", "dart"},
        {".ebnf", "ebnf"},
        {".erl", "erlang"},
        {".ex", "elixir"},
        {".hpp", "cpp"},
        {".go", "go"},
        {".groovy", "groovy"},
        {".h", "c"},
        {".hs", "haskell"},
        {".html", "html"},
        {".java", "java"},
        {".js", "javascript"},
        {".json", "json"},
        {".kt", "kotlin"},
        {".lua", "lua"},
        {".m6r", "metaphor"},
        {".m", "objectivec"},
        {".md", "markdown"},
        {".mm", "objectivec"},
        {".php", "php"},
        {".pl", "perl"},
        {".py", "python"},
        {".r", "r"},
        {".rkt", "racket"},
        {".rb", "ruby"},
        {".rs", "rust"},
        {".scala", "scala"},
        {".sh", "bash"},
        {".sql", "sql"},
        {".swift", "swift"},
        {".ts", "typescript"},
        {".vb", "vbnet"},
        {".vbs", "vbscript"},
        {".xml", "xml"},
        {".yaml", "yaml"},
        {".yml", "yaml"}
    };

    std::string extension = filename_.substr(filename_.find_last_of('.'));
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Look up the extension in the map
    auto it = extensionToLanguage.find(extension);
    if (it != extensionToLanguage.end()) {
        return it->second;
    }

    // If we can't find a match default to plaintext.
    return "plaintext";
}

auto EmbedLexer::getNextToken() -> Token {
    if (emitFilename_) {
        emitFilename_ = false;
        currentToken_ = Token(TokenType::TEXT, "File: " + filename_, "", filename_, 0, 1);
        return currentToken_;
    }

    if (emitFormatDelimeter_) {
        emitFormatDelimeter_ = false;
        currentToken_ = Token(TokenType::TEXT, "```" + getLanguageFromFilename(), "", filename_, 0, 1);
        return currentToken_;
    }

    if (emitEndOfFile_) {
        currentToken_ = Token(TokenType::END_OF_FILE, "", line_, filename_, currentLine_, 1);
        return currentToken_;
    }

    // Get the next token.
    while (true) {
        // If we've hit the end of the file then emit another format delimeter before the EOF.
        if (position_ >= input_.size()) {
            currentToken_ = Token(TokenType::TEXT, "```", "", filename_, currentLine_, 1);
            emitEndOfFile_ = true;
            break;
        }

        char ch = input_[position_];

        // If we have a new line then get the next one.
        if (ch == '\n') {
            // If we've not seen any non-whitespace characters and we're in a text block then emit a blank line.
            if (!seenNonWhitespaceCharacters_) {
                seenNonWhitespaceCharacters_ = true;
                currentToken_ = Token(TokenType::TEXT, "", line_, filename_, currentLine_, 1);
                return currentToken_;
            }

            consumeNewline();
            updateEndOfLine();
            seenNonWhitespaceCharacters_ = false;
            continue;
        }

        seenNonWhitespaceCharacters_ = true;
        currentToken_ = readText();
        break;
    }

    return currentToken_;
}
