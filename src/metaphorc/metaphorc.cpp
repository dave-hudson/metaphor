#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include "Parser.hpp"  // Assuming the Parser class is in Parser.hpp

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <file>\n"
        << "metaphorc: Metaphor compiler" << std::endl;
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

    return 0;
}

