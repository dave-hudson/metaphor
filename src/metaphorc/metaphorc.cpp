#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include "Parser.hpp"  // Assuming the Parser class is in Parser.hpp

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <file>\n"
        << "metaphorc: Metaphor compiler" << std::endl;
}

void recurse(const ASTNode& node, int level, std::string section) {
    std::cout << std::string(level * 2, ' ') << section << std::endl;
    int index = 0;
    for (const auto& child : node.childNodes_) {
        if (child->tokenType_ == TokenType::TEXT) {
            std::cout << std::string((level + 1) * 2, ' ') << child->value_ << std::endl;
        }

        if (child->tokenType_ == TokenType::REQUIRE) {
            index++;
            recurse(*child, level + 1, section + "." + std::to_string(index));
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        return 1;
    }

    std::string filePath = argv[1];

    std::cout << "Processing file: " << filePath << std::endl;
    Parser parser;
    auto res = parser.parse(filePath);

    if (!res) {
        std::vector<std::string> errorMessages = parser.getSyntaxErrors();
        for (std::string s : errorMessages) {
            std::cerr << "----------------\n" << s;
        }

        std::cerr << "----------------\n";
        return -1;
    }

    auto syntaxTree = parser.getSyntaxTree();
    recurse(*syntaxTree, 0, "1");

    return 0;
}

