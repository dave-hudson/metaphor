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
    bool inFormatedSection = false;

    while (i < node.childNodes_.size()) {
        auto& child = node.childNodes_[i];

        // If we have anything other than a text node then simply recurse.
        if (child->tokenType_ != TokenType::TEXT) {
            simplifyText(*child);
            i++;
            continue;
        }

        // If we're not processing a formatted text block then any blank lines we encounter here
        // can just be eaten!
        if (!inFormatedSection) {
            if (child->value_.length() == 0) {
                node.childNodes_.erase(node.childNodes_.begin() + i);
                i++;
                continue;
            }
        }

        // We have a text node.  If we don't have a sibling then we can't look to merge anything.
        if (i == node.childNodes_.size() - 1) {
            i++;
            continue;
        }

        // Do we have a formatted code delimeter?  If yes then track that.
        if (child->value_.substr(0, 3) == "```") {
            inFormatedSection = true;
        }

        // If our sibling isn't a text node we can't merge it.
        auto& sibling = node.childNodes_[i + 1];
        if (sibling->tokenType_ != TokenType::TEXT) {
            inFormatedSection = false;
            i++;
            continue;
        }

        // Is our sibling a formatted code delimeter?
        if (sibling->value_.substr(0, 3) == "```") {
            // If we're in a formatted section then this is ending that block.
            if (inFormatedSection) {
                child->value_ += "\n" + sibling->value_;
                node.childNodes_.erase(node.childNodes_.begin() + i + 1);
                i += 2;
                inFormatedSection = false;
                continue;
            }

            // We're going to start a new formatted block.
            i++;
            continue;
        }

        // If we're in a formatted text section then apply a newline and merge these two elements.
        if (inFormatedSection) {
            child->value_ += "\n" + sibling->value_;
            node.childNodes_.erase(node.childNodes_.begin() + i + 1);
            continue;
        }

        // If our next text is an empty line then this indicates the end of a paragraph.
        if (sibling->value_.length() == 0) {
            node.childNodes_.erase(node.childNodes_.begin() + i + 1);
            i++;
            continue;
        }

        child->value_ += " " + sibling->value_;
        node.childNodes_.erase(node.childNodes_.begin() + i + 1);
    }
}

void recurse(const ASTNode& node, std::string section, std::ostream& out) {
    switch (node.tokenType_) {
    case TokenType::TEXT:
        out << node.value_ << std::endl << std::endl;
        return;

    case TokenType::PRODUCT:
    case TokenType::TRAIT:
    case TokenType::EXAMPLE:
        if (node.childNodes_.size()) {
            const auto& childToken = node.childNodes_[0];
            if (childToken->tokenType_ == TokenType::KEYWORD_TEXT) {
                out << section << " " << childToken->value_ << std::endl << std::endl;
                break;
            }
        }

        out << section << std::endl << std::endl;
        break;

    default:
        break;
    }

    int index = 0;
    for (const auto& child : node.childNodes_) {
        if (child->tokenType_ == TokenType::TRAIT ||
                child->tokenType_ == TokenType::EXAMPLE) {
            index++;
        }

        recurse(*child, section + "." + std::to_string(index), out);
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
    recurse(*syntaxTree, "1", *outStream);

    return 0;
}
