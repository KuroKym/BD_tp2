#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <vector>
#include <fstream>

class Node {
public:
    int order;
    bool check_leaf;
    std::vector<int> values;
    std::vector<std::vector<void*>> keys;
    std::vector<Node*> children;
    Node* nextKey;
    Node* parent;

    Node(int order);
    void insert_at_leaf(int value, void* key);
};

class BplusTree {
public:
    Node* root;
    int order;

    BplusTree(int order);
    Node* search(int value);
    void insert(int value, void* key);
    bool find(int value, void* key);
    void* searchKey(int value);
    void insert_in_parent(Node* node, int value, Node* newLeaf);
    void split(Node* parentNode);
    void printLeaves();
    void printTree(Node* node, int level = 0);

    // Funções de salvar e carregar a árvore
    void saveTree(std::ofstream& file, Node* node);
    void saveToFile(const std::string& filename);
    void loadTree(std::ifstream& file, Node*& node, int order);
    void loadFromFile(const std::string& filename, int order);
};

#endif // BTREE_H
