#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include "Parser.hpp"  // Assuming the Parser class is in Parser.hpp

void processFile(const std::string& filePath) {
    std::cout << "Processing file: " << filePath << std::endl;
    Parser parser;  // Initialize the Parser
    parser.parse(filePath);  // Start parsing the initial file
}

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " <file>\n"
        << "metaphorc: Metaphor compiler" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string filePath = argv[1];

    try {
        processFile(filePath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

