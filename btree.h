#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>

using namespace std;

// Node creation
class Node {
public:
    int order;
    vector<int> values;               // IDs
    vector<Node*> children;           // For internal nodes
    vector<vector<void*>> keys;       // Pointers to blocks for leaf nodes
    Node* nextKey;
    Node* parent;
    bool check_leaf;

    Node(int order) {
        this->order = order;
        this->nextKey = nullptr;
        this->parent = nullptr;
        this->check_leaf = false;
    }
    
    void insert_at_leaf(Node* leaf, int value, void* key);
};

class BplusTree {
public:
    Node* root;

    BplusTree(int order) {
        root = new Node(order);
        root->check_leaf = true;
    }
    
    void insert(int value, void* key);
    Node* search(int value);
    bool find(int value, void* key);
    void insert_in_parent(Node* node, int value, Node* newLeaf);
    void printTree(Node* node);
    void saveTree(ofstream& file, Node* node);
    void saveToFile(const string& filename);
    void loadTree(ifstream& file, Node*& node, int order);
    void loadFromFile(const string& filename, int order);
    void* searchKey(int value);
    void printLeaves();
};

#endif // BTREE_H
