#ifndef __ASTNODE_HPP
#define __ASTNODE_HPP

#include <string>
#include <vector>

#include "Token.hpp"

class ASTNode {
public:
    TokenType tokenType_;
    std::string value_;
    int line_;
    int column_;
    ASTNode* parentNode_;
    std::vector<std::unique_ptr<ASTNode>> childNodes_;

    ASTNode(const Token& token);
    auto addChild(std::unique_ptr<ASTNode> child) -> void;
    auto printTree(int level = 0) const -> void;
};

#endif // __ASTNODE_HPP
