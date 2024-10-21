#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

class BPlusTreeNode {
public:
    bool isLeaf;
    std::vector<std::pair<int, int>> keys;  // pair of id and pos
    std::vector<int> children;              // IDs dos nós filhos
    int nodeID;                             // ID do nó atual
    int next;                               // ID do próximo nó folha

    BPlusTreeNode(bool leaf, int id) : isLeaf(leaf), nodeID(id), next(-1) {}

    // Funções de I/O
    void saveNode();
    static BPlusTreeNode* loadNode(int id);
};

// Função para salvar o nó no disco
void BPlusTreeNode::saveNode() {
    std::ofstream file("bplustree_nodes.bin", std::ios::binary | std::ios::in | std::ios::out);
    file.seekp(nodeID * sizeof(BPlusTreeNode), std::ios::beg);
    file.write(reinterpret_cast<const char*>(this), sizeof(BPlusTreeNode));
    file.close();
}

// Função para carregar o nó do disco
BPlusTreeNode* BPlusTreeNode::loadNode(int id) {
    std::ifstream file("bplustree_nodes.bin", std::ios::binary);
    file.seekg(id * sizeof(BPlusTreeNode), std::ios::beg);
    BPlusTreeNode* node = new BPlusTreeNode(true, id);
    file.read(reinterpret_cast<char*>(node), sizeof(BPlusTreeNode));
    file.close();
    return node;
}

class BPlusTree {
private:
    int minDegree;
    int nodeCounter;  // Contador para dar IDs únicos aos nós
    int rootID;

    void insertNonFull(int nodeID, std::pair<int, int> key);
    void splitChild(int parentID, int index, int childID);
    void traverse(int nodeID);

public:
    BPlusTree(int degree) : minDegree(degree), nodeCounter(0), rootID(-1) {}

    void insert(int id, int pos);
    void display();
    void saveToFile(std::ofstream& file);
    void loadFromFile(std::ifstream& file);
};

void BPlusTree::insert(int id, int pos) {
    std::pair<int, int> key = std::make_pair(id, pos);
    if (rootID == -1) {
        rootID = nodeCounter++;
        BPlusTreeNode* root = new BPlusTreeNode(true, rootID);
        root->keys.push_back(key);
        root->saveNode();
    } else {
        BPlusTreeNode* root = BPlusTreeNode::loadNode(rootID);
        if (root->keys.size() == 2 * minDegree - 1) {
            int newRootID = nodeCounter++;
            BPlusTreeNode* newRoot = new BPlusTreeNode(false, newRootID);
            newRoot->children.push_back(rootID);
            splitChild(newRootID, 0, rootID);
            rootID = newRootID;
            newRoot->saveNode();
        }
        insertNonFull(rootID, key);
    }
}

void BPlusTree::insertNonFull(int nodeID, std::pair<int, int> key) {
    BPlusTreeNode* node = BPlusTreeNode::loadNode(nodeID);
    int i = node->keys.size() - 1;
    if (node->isLeaf) {
        node->keys.push_back(std::make_pair(0, 0));
        while (i >= 0 && node->keys[i].first > key.first) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->saveNode();
    } else {
        while (i >= 0 && node->keys[i].first > key.first) {
            i--;
        }
        i++;
        BPlusTreeNode* child = BPlusTreeNode::loadNode(node->children[i]);
        if (child->keys.size() == 2 * minDegree - 1) {
            splitChild(nodeID, i, node->children[i]);
            if (node->keys[i].first < key.first) {
                i++;
            }
        }
        insertNonFull(node->children[i], key);
    }
    delete node;
}

void BPlusTree::splitChild(int parentID, int index, int childID) {
    BPlusTreeNode* parent = BPlusTreeNode::loadNode(parentID);
    BPlusTreeNode* child = BPlusTreeNode::loadNode(childID);

    int newChildID = nodeCounter++;
    BPlusTreeNode* newChild = new BPlusTreeNode(child->isLeaf, newChildID);
    parent->children.insert(parent->children.begin() + index + 1, newChildID);
    parent->keys.insert(parent->keys.begin() + index, child->keys[minDegree - 1]);

    newChild->keys.assign(child->keys.begin() + minDegree, child->keys.end());
    child->keys.resize(minDegree - 1);

    if (!child->isLeaf) {
        newChild->children.assign(child->children.begin() + minDegree, child->children.end());
        child->children.resize(minDegree);
    }

    if (child->isLeaf) {
        newChild->next = child->next;
        child->next = newChildID;
    }

    parent->saveNode();
    child->saveNode();
    newChild->saveNode();

    delete parent;
    delete child;
    delete newChild;
}

void BPlusTree::display() {
    traverse(rootID);
}

void BPlusTree::traverse(int nodeID) {
    if (nodeID == -1) return;

    BPlusTreeNode* node = BPlusTreeNode::loadNode(nodeID);
    for (int i = 0; i < node->keys.size(); i++) {
        if (!node->isLeaf) {
            traverse(node->children[i]);
        }
        std::cout << "(" << node->keys[i].first << ", " << node->keys[i].second << ") ";
    }
    if (!node->isLeaf) {
        traverse(node->children[node->keys.size()]);
    }
    delete node;
}


// Função para salvar a árvore inteira no arquivo
void BPlusTree::saveToFile(std::ofstream& file) {
    // Primeiro, escrevemos o ID da raiz e o contador de nós
    file.write(reinterpret_cast<const char*>(&rootID), sizeof(rootID));
    file.write(reinterpret_cast<const char*>(&nodeCounter), sizeof(nodeCounter));

    // Vamos então percorrer todos os nós da árvore e salvar cada um
    std::vector<int> nodeQueue = {rootID};  // Começamos pela raiz
    while (!nodeQueue.empty()) {
        int currentNodeID = nodeQueue.back();
        nodeQueue.pop_back();

        BPlusTreeNode* currentNode = BPlusTreeNode::loadNode(currentNodeID);

        // Salvar o nó atual
        currentNode->saveNode();

        // Se o nó não for uma folha, adicionamos seus filhos à fila para salvar
        if (!currentNode->isLeaf) {
            for (int childID : currentNode->children) {
                nodeQueue.push_back(childID);
            }
        }

        delete currentNode;
    }
}


// Função para carregar a árvore inteira do arquivo
void BPlusTree::loadFromFile(std::ifstream& file) {
    // Primeiro, lemos o ID da raiz e o contador de nós
    file.read(reinterpret_cast<char*>(&rootID), sizeof(rootID));
    file.read(reinterpret_cast<char*>(&nodeCounter), sizeof(nodeCounter));

    // Vamos então carregar todos os nós da árvore
    std::vector<int> nodeQueue = {rootID};  // Começamos pela raiz
    while (!nodeQueue.empty()) {
        int currentNodeID = nodeQueue.back();
        nodeQueue.pop_back();

        BPlusTreeNode* currentNode = BPlusTreeNode::loadNode(currentNodeID);

        // Se o nó não for uma folha, adicionamos seus filhos à fila para carregar
        if (!currentNode->isLeaf) {
            for (int childID : currentNode->children) {
                nodeQueue.push_back(childID);
            }
        }

        delete currentNode;
    }
}
