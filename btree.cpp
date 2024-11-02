
#include "btree.h"
 
// Insert at the leaf
void Node::insert_at_leaf(Node* leaf, int value, void* key) {
    if (!values.empty()) {
        for (int i = 0; i < values.size(); i++) {
            if (value == values[i]) {
                keys[i].push_back(key);
                break;
            } else if (value < values[i]) {
                values.insert(values.begin() + i, value);
                keys.insert(keys.begin() + i, vector<void*>{key});
                break;
            } else if (i + 1 == values.size()) {
                values.push_back(value);
                keys.push_back(vector<void*>{key});
                break;
            }
        }
    } else {
        values.push_back(value);
        keys.push_back(vector<void*>{key});
    }
}

// Insert operation
void BplusTree::insert(int value, void* key) {
    Node* leafNode = search(value);
    leafNode->insert_at_leaf(leafNode, value, key);

    if (leafNode->values.size() == leafNode->order) {
        Node* newLeaf = new Node(leafNode->order);
        newLeaf->check_leaf = true;
        newLeaf->parent = leafNode->parent;

        int mid = ceil(leafNode->order / 2.0) - 1;
        newLeaf->values.assign(leafNode->values.begin() + mid + 1, leafNode->values.end());
        newLeaf->keys.assign(leafNode->keys.begin() + mid + 1, leafNode->keys.end());
        newLeaf->nextKey = leafNode->nextKey;

        leafNode->values.resize(mid + 1);
        leafNode->keys.resize(mid + 1);
        leafNode->nextKey = newLeaf;

        insert_in_parent(leafNode, newLeaf->values[0], newLeaf);
    }
}

// Search operation for finding the correct leaf node
Node* BplusTree::search(int value) {
    Node* currentNode = root;
    while (!currentNode->check_leaf) {
        for (int i = 0; i < currentNode->values.size(); i++) {
            if (value == currentNode->values[i]) {
                currentNode = currentNode->children[i + 1];
                break;
            } else if (value < currentNode->values[i]) {
                currentNode = currentNode->children[i];
                break;
            } else if (i + 1 == currentNode->values.size()) {
                currentNode = currentNode->children[i + 1];
                break;
            }
        }
    }
    return currentNode;
}

// Find the node with a specific value and key
bool BplusTree::find(int value, void* key) {
    Node* leaf = search(value);
    for (int i = 0; i < leaf->values.size(); i++) {
        if (leaf->values[i] == value) {
            for (void* storedKey : leaf->keys[i]) {
                if (storedKey == key) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Insert at the parent node
void BplusTree::insert_in_parent(Node* node, int value, Node* newLeaf) {
    if (root == node) {
        Node* newRoot = new Node(node->order);
        newRoot->values.push_back(value);
        newRoot->children.push_back(node);
        newRoot->children.push_back(newLeaf);
        root = newRoot;
        node->parent = newRoot;
        newLeaf->parent = newRoot;
        return;
    }

    Node* parentNode = node->parent;
    for (int i = 0; i < parentNode->children.size(); i++) {
        if (parentNode->children[i] == node) {
            parentNode->values.insert(parentNode->values.begin() + i, value);
            parentNode->children.insert(parentNode->children.begin() + i + 1, newLeaf);

            if (parentNode->children.size() > parentNode->order) {
                Node* newParent = new Node(parentNode->order);
                newParent->parent = parentNode->parent;
                int mid = ceil(parentNode->order / 2.0) - 1;

                newParent->values.assign(parentNode->values.begin() + mid + 1, parentNode->values.end());
                newParent->children.assign(parentNode->children.begin() + mid + 1, parentNode->children.end());
                int upValue = parentNode->values[mid];

                parentNode->values.resize(mid);
                parentNode->children.resize(mid + 1);

                insert_in_parent(parentNode, upValue, newParent);
            }
            break;
        }
    }
}

// Display the tree
void BplusTree::printTree(Node* node, int depth) {
    if (node == nullptr) return;

    // Imprimir os valores do nó
    for (int i = 0; i < node->values.size(); i++) {
        // Imprimir a profundidade da árvore
        cout << node->values[i] << " "; // 4 espaços para cada nível
    }
    cout << endl;

    // Se o nó não for uma folha, imprimir os filhos
    if (!node->check_leaf) {
        for (int i = 0; i <= node->values.size(); i++) {
            printTree(node->children[i], depth + 1);
        }
    }
}

// Função para salvar a árvore em um arquivo
void BplusTree::saveTree(ofstream& file, Node* node) {
    if (!node) return;

    // Salva informações do nó
    file.write((char*)&node->check_leaf, sizeof(node->check_leaf));
    int numValues = node->values.size();
    file.write((char*)&numValues, sizeof(numValues));

    for (int value : node->values) {
        file.write((char*)&value, sizeof(value));
    }

    if (node->check_leaf) {
        long nextKeyAddr = (node->nextKey) ? reinterpret_cast<long>(node->nextKey) : -1;
        file.write((char*)&nextKeyAddr, sizeof(nextKeyAddr));
    } else {
        int numChildren = node->children.size();
        file.write((char*)&numChildren, sizeof(numChildren));
        for (Node* child : node->children) {
            long childAddr = reinterpret_cast<long>(child);
            file.write((char*)&childAddr, sizeof(childAddr));
        }
    }

    // Salvar recursivamente os filhos
    for (Node* child : node->children) {
        saveTree(file, child);
    }
}

// Função para salvar a árvore em um arquivo
void BplusTree::saveToFile(const string& filename) {
    ofstream file(filename, ios::binary);
    if (file.is_open()) {
        saveTree(file, root);
        file.close();
        cout << "Árvore salva com sucesso em " << filename << endl;
    } else {
        cout << "Erro ao abrir o arquivo." << endl;
    }
}

// Função para carregar a árvore de um arquivo
void BplusTree::loadTree(ifstream& file, Node*& node, int order) {
    if (!file.is_open() || file.eof()) return;

    // Ler informações do nó
    bool isLeaf;
    file.read((char*)&isLeaf, sizeof(isLeaf));
    
    node = new Node(order);
    node->check_leaf = isLeaf;

    int numValues;
    file.read((char*)&numValues, sizeof(numValues));
    node->values.resize(numValues);
    
    for (int i = 0; i < numValues; ++i) {
        file.read((char*)&node->values[i], sizeof(int));
    }

    if (isLeaf) {
        long nextKeyAddr;
        file.read((char*)&nextKeyAddr, sizeof(nextKeyAddr));
        node->nextKey = reinterpret_cast<Node*>(nextKeyAddr);
    } else {
        int numChildren;
        file.read((char*)&numChildren, sizeof(numChildren));
        node->children.resize(numChildren);

        for (int i = 0; i < numChildren; ++i) {
            long childAddr;
            file.read((char*)&childAddr, sizeof(childAddr));
            node->children[i] = reinterpret_cast<Node*>(childAddr);
        }
    }

    // Carregar recursivamente os filhos
    for (Node* child : node->children) {
        loadTree(file, child, order);
    }
}

// Função para carregar a árvore de um arquivo
void BplusTree::loadFromFile(const string& filename, int order) {
    ifstream file(filename, ios::binary);
    if (file.is_open()) {
        loadTree(file, root, order);
        file.close();
        cout << "Árvore carregada com sucesso de " << filename << endl;
    } else {
        cout << "Erro ao abrir o arquivo." << endl;
    }
}
   

/*int main() {
    int order = 3;
    BplusTree bplustree(order);
    BplusTree bplustree2(order);

    int id = 5;
    void* blockPtr = reinterpret_cast<void*>(0x1000);
    bplustree.insert(id, blockPtr);
    id = 52;
    blockPtr = reinterpret_cast<void*>(0x1200);
    bplustree.insert(id, blockPtr);
    id = 2;
    blockPtr = reinterpret_cast<void*>(0x1400); // Example block pointer

    bplustree.insert(id, blockPtr);
    bplustree.saveToFile("index.bin");
    cout << "Saved" << endl;
    bplustree2.loadFromFile("index.bin", order);
    cout << "Loaded" << endl;
    bplustree2.printTree(bplustree2.root, 3);
    if (bplustree2.find(id, blockPtr)) {
        cout << "Found" << endl;
    } else {
        cout << "Not found" << endl;
    }

    return 0;
}*/
