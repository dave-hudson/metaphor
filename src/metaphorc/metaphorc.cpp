#include <iostream>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <getopt.h>
#include "Parser.hpp"

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [options] <file>\n"
        << "Options:\n"
        << "  -h, --help                Print this help message\n"
        << "  -o, --outputFile <file>   Specify output file\n"
        << "  -d, --debug               Generate debug output\n"
        << std::endl;
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
    std::string outputFile;
    bool debug = false;

    const char* const short_opts = "ho:d";
    const option long_opts[] = {
        {"help", no_argument, nullptr, 'h'},
        {"outputFile", required_argument, nullptr, 'o'},
        {"debug", no_argument, nullptr, 'd'},
        {nullptr, no_argument, nullptr, 0}
    };

    while (true) {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (opt == -1) break;

        switch (opt) {
        case 'h':
            printUsage(argv[0]);
            return 0;

        case 'o':
            outputFile = optarg;
            break;

        case 'd':
            debug = true;
            break;

        case '?':
            printUsage(argv[0]);
            return 1;

        default:
            printUsage(argv[0]);
            return 1;
        }
    }

    if (optind >= argc) {
        std::cerr << "Error: No input file specified.\n";
        printUsage(argv[0]);
        return 1;
    }

    std::string filePath = argv[optind];

    if (debug) {
        std::cerr << "Debug mode is ON\n";
    }

    if (!outputFile.empty()) {
        std::cerr << "Output file: " << outputFile << std::endl;
    }

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
