#include <iostream>

#include "ASTNode.hpp"

ASTNode::ASTNode(const Token& token) :
        tokenType_(token.type),
        value_(token.value),
        line_(token.line),
        column_(token.column),
        parentNode_(NULL) {
}

// Method to add a child
auto ASTNode::addChild(std::unique_ptr<ASTNode> child) -> void {
    child->parentNode_ = this;
    childNodes_.push_back(std::move(child));
}

// Method to print the tree for debugging
auto ASTNode::printTree(int level) const -> void {
    std::cout << std::string(level * 2, ' ') << value_ << std::endl;
    for (const auto& child : childNodes_) {
        child->printTree(level + 1);
    }
}

