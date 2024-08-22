#include <iostream>
#include <fstream>
#include <unordered_map>

#include "CodeLexer.hpp"

CodeLexer::CodeLexer(const std::string& filename) :
        Lexer(filename),
        emitFormatDelimeter_(true),
        emitEndOfFile_(false) {
}

auto CodeLexer::updateEndOfLine() -> void {
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

auto CodeLexer::readText() -> Token {
    position_ = endOfLine_;
    return Token(TokenType::TEXT, line_.substr(0, endOfLine_ - startOfLine_), line_, filename_, currentLine_, 1);
}

auto CodeLexer::getLanguageFromFilename() -> std::string {
    static const std::unordered_map<std::string, std::string> extensionToLanguage = {
        {".cpp", "cpp"},
        {".hpp", "cpp"},
        {".h", "cpp"},
        {".c", "c"},
        {".cs", "csharp"},
        {".java", "java"},
        {".py", "python"},
        {".js", "javascript"},
        {".ts", "typescript"},
        {".rb", "ruby"},
        {".php", "php"},
        {".html", "html"},
        {".css", "css"},
        {".swift", "swift"},
        {".m", "objectivec"},
        {".mm", "objectivec"},
        {".go", "go"},
        {".rs", "rust"},
        {".kt", "kotlin"},
        {".sh", "bash"},
        {".bash", "bash"},
        {".r", "r"},
        {".sql", "sql"},
        {".xml", "xml"},
        {".json", "json"},
        {".yaml", "yaml"},
        {".yml", "yaml"},
        {".pl", "perl"},
        {".lua", "lua"},
        {".scala", "scala"},
        {".hs", "haskell"},
        {".erl", "erlang"},
        {".ex", "elixir"},
        {".clj", "clojure"},
        {".groovy", "groovy"},
        {".dart", "dart"},
        {".rkt", "racket"},
        {".vb", "vbnet"},
        {".vbs", "vbscript"}
    };

    std::string extension = filename_.substr(filename_.find_last_of('.'));
    
    // Look up the extension in the map
    auto it = extensionToLanguage.find(extension);
    if (it != extensionToLanguage.end()) {
        return it->second;
    }

    // If we can't find a match default to plaintext.
    return "plaintext";
}

auto CodeLexer::getNextToken() -> Token {
    if (emitFormatDelimeter_) {
        emitFormatDelimeter_ = false;
        currentToken_ = Token(TokenType::TEXT, "```" + getLanguageFromFilename(), line_, filename_, 0, 1);
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
            currentToken_ = Token(TokenType::TEXT, "```", line_, filename_, currentLine_, 1);
            emitEndOfFile_ = true;
            break;
        }

        char ch = input_[position_];

        // If we have a new line then get the next one.
        if (ch == '\n') {
            consumeNewline();
            updateEndOfLine();
            continue;
        }

        currentToken_ = readText();
        break;
    }

    return currentToken_;
}
