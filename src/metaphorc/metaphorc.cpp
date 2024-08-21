#include <iostream>
#include <fstream>
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

void simplifyText(ASTNode& node) {
    size_t i = 0;
    while (i < node.childNodes_.size()) {
        auto& child = node.childNodes_[i];

        // If we have anything other than a text node then simply recurse.
        if (child->tokenType_ != TokenType::TEXT) {
            simplifyText(*child);
            i++;
            continue;
        }

        // We have a text node.  Look to see if we have another one following it, in which case we
        // may want to merge these two.
        if (i == node.childNodes_.size() - 1) {
            i++;
            continue;
        }

        auto& sibling = node.childNodes_[i + 1];
        if (sibling->tokenType_ != TokenType::TEXT) {
            i++;
            continue;
        }

        // If our next text is an empty line then erase it, but move on from this block.
        if (sibling->value_.length() == 0) {
            node.childNodes_.erase(node.childNodes_.begin() + i + 1);
            i++;
            continue;
        }

        child->value_ += " " + sibling->value_;
        node.childNodes_.erase(node.childNodes_.begin() + i + 1);
    }
}

void recurse(const ASTNode& node, int level, std::string section, std::ostream& out) {
    switch (node.tokenType_) {
    case TokenType::TEXT:
        out << std::string(level * 2, ' ') << node.value_ << std::endl;
        return;

    case TokenType::REQUIRE:
    case TokenType::EXAMPLE:
    case TokenType::STORY:
        out << std::string(level * 2, ' ') << section << std::endl;
        break;

    case TokenType::AS:
        out << std::string(level * 2, ' ') << "As" << std::endl;
        break;

    case TokenType::I:
        out << std::string(level * 2, ' ') << "I" << std::endl;
        break;

    case TokenType::SO:
        out << std::string(level * 2, ' ') << "so" << std::endl;
        break;

    case TokenType::GIVEN:
        out << std::string(level * 2, ' ') << "Given" << std::endl;
        break;

    case TokenType::WHEN:
        out << std::string(level * 2, ' ') << "when" << std::endl;
        break;

    case TokenType::THEN:
        out << std::string(level * 2, ' ') << "then" << std::endl;
        break;

    default:
        break;
    }

    int index = 0;
    for (const auto& child : node.childNodes_) {
        if (child->tokenType_ == TokenType::REQUIRE ||
                child->tokenType_ == TokenType::EXAMPLE ||
                child->tokenType_ == TokenType::STORY) {
            index++;
        }

        recurse(*child, level + 1, section + "." + std::to_string(index), out);
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

    std::ostream* outStream = &std::cout;
    std::ofstream outFile;

    if (!outputFile.empty()) {
        outFile.open(outputFile);
        if (!outFile) {
            std::cerr << "Error: Could not open output file " << outputFile << " for writing.\n";
            return 1;
        }

        outStream = &outFile;
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
    simplifyText(*syntaxTree);
    recurse(*syntaxTree, 0, "1", *outStream);

    return 0;
}
