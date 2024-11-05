#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

class Node {
public:
    int order;
    bool check_leaf;
    std::vector<int> values;
    std::vector<std::vector<std::streampos>> keys;  // Alterado de void* para std::streampos
    std::vector<Node*> children;
    Node* nextKey;
    Node* parent;

    Node(int order);
    void insert_at_leaf(int value, std::streampos key);  // Alterado de void* para std::streampos
};

class BplusTree {
public:
    Node* root;
    int order;

    BplusTree(int order);
    Node* search(int value);
    void insert(int value, std::streampos key);  // Alterado de void* para std::streampos
    bool find(int value, std::streampos key);  // Alterado de void* para std::streampos
    std::streampos searchKey(int value);  // Alterado de void* para std::streampos
    void insert_in_parent(Node* node, int value, Node* newLeaf);
    void split(Node* parentNode);
    void printLeaves();
    void printTree(Node* node, int level = 0);

    // Funções de salvar e carregar a árvore
    void saveTree(std::ofstream& file, Node* node);
    void saveToFile(const std::string& filename);
    void loadTree(std::ifstream& file, Node*& node, int order);
    void loadFromFile(const std::string& filename);
};

#endif // BTREE_H
