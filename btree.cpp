
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
    std::cout << "Inserindo valor: " << value << std::endl;
    Node* leafNode = search(value);
    std::cout << "Nó folha encontrado para inserção: ";
    for (const auto& val : leafNode->values) {
        std::cout << val << " ";
    }
    std::cout << std::endl;

    leafNode->insert_at_leaf(leafNode, value, key);

    if (leafNode->values.size() == leafNode->order) {
        std::cout << "Nó folha está cheio, realizando split." << std::endl;
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

        std::cout << "Nó folha dividido. Novo nó folha contém valores: ";
        for (const auto& val : newLeaf->values) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

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
// Search for a value and return its key
void* BplusTree::searchKey(int value) {
    Node* leaf = search(value);
    for (int i = 0; i < leaf->values.size(); i++) {
        if (leaf->values[i] == value) {
            if (!leaf->keys[i].empty()) {
                return leaf->keys[i][0]; // Return the first key found
            }
        }
    }
    return nullptr; // Return nullptr if the value or key is not found
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

void BplusTree::printLeaves() {
    Node* current = root;

    // Navegar até o nó folha mais à esquerda
    while (current != nullptr && !current->check_leaf) {
        std::cout << "Navegando para o filho mais à esquerda do nó com valores: ";
        for (const auto& value : current->values) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
        current = current->children[0]; // Move para o filho mais à esquerda
    }

    // Imprimir todos os valores das folhas
    while (current != nullptr) {
        std::cout << "Valores da folha: ";
        for (const auto& value : current->values) {
            std::cout << value << " ";
        }
        std::cout << std::endl;

        // Avançar para a próxima folha encadeada
        std::cout << "Avançando para a próxima folha." << std::endl;
        current = current->nextKey; // Usar nextKey para ir à próxima folha
    }
}



// Display the tree
void BplusTree::printTree(Node* node) {
        if (node == nullptr) return;
        for (int i = 0; i < node->values.size(); i++) {
            cout << node->values[i] << " ";
        }
        cout << endl;
        if (!node->check_leaf) {
            for (int i = 0; i <= node->values.size(); i++) {
                printTree(node->children[i]);
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
    if (!file.is_open() || file.eof()) {
        std::cout << "Arquivo não aberto ou EOF atingido. Encerrando carregamento." << std::endl;
        return;
    }

    // Ler informações do nó
    bool isLeaf;
    file.read((char*)&isLeaf, sizeof(isLeaf));
    std::cout << "Carregando nó: " << (isLeaf ? "Folha" : "Interno") << std::endl;

    node = new Node(order);
    node->check_leaf = isLeaf;

    int numValues;
    file.read((char*)&numValues, sizeof(numValues));
    node->values.resize(numValues);
    std::cout << "Número de valores no nó: " << numValues << std::endl;

    for (int i = 0; i < numValues; ++i) {
        file.read((char*)&node->values[i], sizeof(int));
        std::cout << "Valor[" << i << "]: " << node->values[i] << std::endl;
    }

    if (isLeaf) {
        long nextKeyAddr;
        file.read((char*)&nextKeyAddr, sizeof(nextKeyAddr));
        node->nextKey = reinterpret_cast<Node*>(nextKeyAddr);
        std::cout << "Endereço do próximo nó chave: " << nextKeyAddr << std::endl;
    } else {
        int numChildren;
        file.read((char*)&numChildren, sizeof(numChildren));
        node->children.resize(numChildren);
        std::cout << "Número de filhos: " << numChildren << std::endl;

        for (int i = 0; i < numChildren; ++i) {
            long childAddr;
            file.read((char*)&childAddr, sizeof(childAddr));
            node->children[i] = reinterpret_cast<Node*>(childAddr);
            std::cout << "Endereço do filho[" << i << "]: " << childAddr << std::endl;
        }
    }

    // Carregar recursivamente os filhos
    for (int i = 0; i < node->children.size(); ++i) {
        loadTree(file, node->children[i], order);
        node->children[i]->parent = node; // Correctly link the parent
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
   

int main() {
    int order = 3;
    BplusTree bplustree(order);
    BplusTree bplustree2(order);

    int id = 1;
    void* blockPtr = reinterpret_cast<void*>(0x1000);
    bplustree.insert(id, blockPtr);
    id = 2;
    blockPtr = reinterpret_cast<void*>(0x1200);
    bplustree.insert(id, blockPtr);
    id = 3;
    blockPtr = reinterpret_cast<void*>(0x1400); // Example block pointer
    bplustree.insert(id, blockPtr);
    id = 4;
    blockPtr = reinterpret_cast<void*>(0x1600); // Example block pointer
    bplustree.insert(id, blockPtr);
    id = 5;
    blockPtr = reinterpret_cast<void*>(0x1800); // Example block pointer
    bplustree.insert(id, blockPtr);
    id = 6;
    blockPtr = reinterpret_cast<void*>(0x2000); // Example block pointer
    bplustree.insert(id, blockPtr);
    id = 7;
    blockPtr = reinterpret_cast<void*>(0x2200); // Example block pointer
    bplustree.insert(id, blockPtr);
    id = 8;
    blockPtr = reinterpret_cast<void*>(0x2400); // Example block pointer
    bplustree.insert(id, blockPtr);
    id = 9;
    blockPtr = reinterpret_cast<void*>(0x2600); // Example block pointer
    bplustree.insert(id, blockPtr);
    id = 10;
    blockPtr = reinterpret_cast<void*>(0x2800); // Example block pointer
    bplustree.insert(id, blockPtr);

    bplustree.saveToFile("index.bin");
    //bplustree2.loadFromFile("index.bin", order);
    bplustree.printTree(bplustree.root);
    //cout << "\n" << endl;
    //cout << bplustree2.root->children[1]->values[0] << endl;

    return 0;
}
