// btree.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>

const int MAX_KEYS = 3; // Número máximo de chaves por nó

struct BPlusTreeNode {
    bool isLeaf;
    std::vector<int> keys; // Chaves
    std::vector<std::shared_ptr<BPlusTreeNode>> children; // Filhos

    BPlusTreeNode(bool isLeaf) : isLeaf(isLeaf) {}
};

// Classe que representa a B+ Tree
class BPlusTree {
public:
    BPlusTree() : root(std::make_shared<BPlusTreeNode>(true)) {}

    void insert(int key);
    void print() const;

private:
    std::shared_ptr<BPlusTreeNode> root;

    void insertNonFull(std::shared_ptr<BPlusTreeNode> node, int key);
    void splitChild(std::shared_ptr<BPlusTreeNode> parent, int index);
    void printNode(const std::shared_ptr<BPlusTreeNode>& node, int depth) const;
};

// Função para inserir uma chave na B+ Tree
void BPlusTree::insert(int key) {
    if (root->keys.size() == MAX_KEYS) {
        // Criar um novo nó raiz
        auto newRoot = std::make_shared<BPlusTreeNode>(false);
        newRoot->children.push_back(root);
        splitChild(newRoot, 0);
        root = newRoot;
    }
    insertNonFull(root, key);
}

// Inserir uma chave em um nó que não está cheio
void BPlusTree::insertNonFull(std::shared_ptr<BPlusTreeNode> node, int key) {
    if (node->isLeaf) {
        node->keys.push_back(key);
        std::sort(node->keys.begin(), node->keys.end()); // Ordenar as chaves
    } else {
        int i = node->keys.size() - 1;
        while (i >= 0 && key < node->keys[i]) {
            --i;
        }
        ++i;
        if (node->children[i]->keys.size() == MAX_KEYS) {
            splitChild(node, i);
            if (key > node->keys[i]) {
                ++i;
            }
        }
        insertNonFull(node->children[i], key);
    }
}

// Função para dividir um nó cheio
void BPlusTree::splitChild(std::shared_ptr<BPlusTreeNode> parent, int index) {
    auto fullNode = parent->children[index];
    auto newNode = std::make_shared<BPlusTreeNode>(fullNode->isLeaf);
    int medianIndex = MAX_KEYS / 2;

    // Mover a chave mediana para o nó pai
    parent->keys.insert(parent->keys.begin() + index, fullNode->keys[medianIndex]);
    parent->children.insert(parent->children.begin() + index + 1, newNode);

    // Mover as chaves e filhos para o novo nó
    newNode->keys.assign(fullNode->keys.begin() + medianIndex + 1, fullNode->keys.end());
    fullNode->keys.resize(medianIndex);
    
    if (!fullNode->isLeaf) {
        newNode->children.assign(fullNode->children.begin() + medianIndex + 1, fullNode->children.end());
        fullNode->children.resize(medianIndex + 1);
    }
}

// Função para imprimir a B+ Tree
void BPlusTree::print() const {
    printNode(root, 0);
}

void BPlusTree::printNode(const std::shared_ptr<BPlusTreeNode>& node, int depth) const {
    if (node != nullptr) {
        for (int i = 0; i < node->keys.size(); ++i) {
            printNode(node->children[i], depth + 1);
            for (int j = 0; j < depth; ++j) {
                std::cout << "  ";
            }
            std::cout << node->keys[i] << std::endl;
        }
        printNode(node->children[node->children.size() - 1], depth + 1);
    }
}

int main() {
    BPlusTree btree;
    
    // Exemplo de inserção de chaves
    btree.insert(5);
    btree.insert(10);
    btree.insert(3);
    btree.insert(6);
    btree.insert(4);
    btree.insert(8);
    
    std::cout << "B+ Tree:" << std::endl;
    btree.print();

    return 0;
}
