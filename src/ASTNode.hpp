#ifndef __ASTNODE_HPP
#define __ASTNODE_HPP

#include <string>
#include <vector>

#include "Token.hpp"

class ASTNode {
public:
    TokenType tokenType;
    std::string value;
    int line;
    int column;
    ASTNode* parentNode;
    std::vector<std::unique_ptr<ASTNode>> childNodes;

    ASTNode(const Token& token);
    auto addChild(std::unique_ptr<ASTNode> child) -> void;
    auto printTree(int level = 0) const -> void;
};

#endif // __ASTNODE_HPP
