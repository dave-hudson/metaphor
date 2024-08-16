#include <iostream>

#include "ASTNode.hpp"

ASTNode::ASTNode(const Token& token) :
        tokenType(token.type),
        value(token.value),
        line(token.line),
        column(token.column),
        parentNode(NULL) {
}

// Method to add a child
auto ASTNode::addChild(std::unique_ptr<ASTNode> child) -> void {
    child->parentNode = this;
    childNodes.push_back(std::move(child));
}

// Method to print the tree for debugging
auto ASTNode::printTree(int level) const -> void {
    std::cout << std::string(level * 2, ' ') << value << std::endl;
    for (const auto& child : childNodes) {
        child->printTree(level + 1);
    }
}

