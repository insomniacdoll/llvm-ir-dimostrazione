#include "ast.h"
#include "tree.h"

#include "lexer.h"
#include "bison_parser.hh"


STree::STree() : root(nullptr) {}

STree::STree(STree &&other) {
    root = std::move(other.root);
    other.root = nullptr;
}

STree &
STree::operator =(STree other) {
    std::swap(root, other.root);
    return *this;
}

STree::~STree() {
    root.reset();
}

void
STree::parse(std::istream &input) {
    // Drop the previous tree instead of leaking it.
    root.reset();

    Lexer lexer(&input);
    // Own the parser by value: it was previously heap-allocated and never
    // freed, leaking one parser per statement.
    bison::Parser parser(lexer, *this);

    parser.parse();
}

void
STree::set_root(FunctionNode *node) {
    root = unique_ptr<ASTNode>(node);
}

void
STree::set_root(PrototypeNode *node) {
    root = unique_ptr<ASTNode>(node);
}
