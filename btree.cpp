#include "btree.h"

Node::Node(int order) : order(order), check_leaf(true), nextKey(nullptr), parent(nullptr) {}

void Node::insert_at_leaf(int value, void* key) {
    if (!values.empty()) {
        for (int i = 0; i < values.size(); i++) {
            if (value == values[i]) {
                keys[i].push_back(key);
                return;
            } else if (value < values[i]) {
                values.insert(values.begin() + i, value);
                keys.insert(keys.begin() + i, std::vector<void*>{key});
                return;
            }
        }
    }
    values.push_back(value);
    keys.push_back(std::vector<void*>{key});
}

BplusTree::BplusTree(int order) : order(order) {
    root = new Node(order);
}

Node* BplusTree::search(int value) {
    Node* currentNode = root;
    while (!currentNode->check_leaf) {
        for (int i = 0; i < currentNode->values.size(); i++) {
            if (value < currentNode->values[i]) {
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

void BplusTree::insert(int value, void* key) {
    std::cout << "Inserindo valor: " << value << std::endl;
    Node* leafNode = search(value);

    leafNode->insert_at_leaf(value, key);

    if (leafNode->values.size() == order) {
        std::cout << "Nó folha está cheio, realizando split." << std::endl;
        Node* newLeaf = new Node(order);
        newLeaf->check_leaf = true;
        newLeaf->parent = leafNode->parent;

        int mid = (order + 1) / 2;
        newLeaf->values.assign(leafNode->values.begin() + mid, leafNode->values.end());
        newLeaf->keys.assign(leafNode->keys.begin() + mid, leafNode->keys.end());
        newLeaf->nextKey = leafNode->nextKey;

        leafNode->values.resize(mid);
        leafNode->keys.resize(mid);
        leafNode->nextKey = newLeaf;

        std::cout << "Nó folha dividido. Novo nó folha contém valores: ";
        for (const auto& val : newLeaf->values) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        insert_in_parent(leafNode, newLeaf->values[0], newLeaf);
    }
}

void BplusTree::insert_in_parent(Node* node, int value, Node* newLeaf) {
    if (root == node) {
        Node* newRoot = new Node(order);
        newRoot->values.push_back(value);
        newRoot->children.push_back(node);
        newRoot->children.push_back(newLeaf);
        newRoot->check_leaf = false;
        root = newRoot;
        node->parent = newRoot;
        newLeaf->parent = newRoot;
        return;
    }

    Node* parentNode = node->parent;
    int insertPos = 0;
    while (insertPos < parentNode->values.size() && value > parentNode->values[insertPos]) {
        insertPos++;
    }

    parentNode->values.insert(parentNode->values.begin() + insertPos, value);
    parentNode->children.insert(parentNode->children.begin() + insertPos + 1, newLeaf);
    newLeaf->parent = parentNode;

    if (parentNode->values.size() == order) {
        split(parentNode);
    }
}

void BplusTree::split(Node* parentNode) {
    Node* newInternal = new Node(order);
    newInternal->check_leaf = false;
    newInternal->parent = parentNode->parent;

    int mid = order / 2;
    int midValue = parentNode->values[mid];

    newInternal->values.assign(parentNode->values.begin() + mid + 1, parentNode->values.end());
    newInternal->children.assign(parentNode->children.begin() + mid + 1, parentNode->children.end());

    for (Node* child : newInternal->children) {
        child->parent = newInternal;
    }

    parentNode->values.resize(mid);
    parentNode->children.resize(mid + 1);

    if (parentNode == root) {
        Node* newRoot = new Node(order);
        newRoot->check_leaf = false;
        newRoot->values.push_back(midValue);
        newRoot->children.push_back(parentNode);
        newRoot->children.push_back(newInternal);
        root = newRoot;
        parentNode->parent = newRoot;
        newInternal->parent = newRoot;
    } else {
        insert_in_parent(parentNode, midValue, newInternal);
    }
}

void BplusTree::printTree(Node* node, int level) {
    if (!node) return;

    std::cout << "Nível " << level << ": ";
    for (const auto& value : node->values) {
        std::cout << value << " ";
    }
    std::cout << std::endl;

    if (!node->check_leaf) {
        for (Node* child : node->children) {
            printTree(child, level + 1);
        }
    }
}

void BplusTree::printLeaves() {
    Node* current = root;
    while (current && !current->check_leaf) {
        current = current->children[0];
    }

    while (current) {
        for (const auto& value : current->values) {
            std::cout << value << " ";
        }
        std::cout << " -> ";
        current = current->nextKey;
    }
    std::cout << "nullptr" << std::endl;
}

void BplusTree::saveTree(std::ofstream& file, Node* node) {
    if (!node) return;
    
    // Salvar o indicador de nó folha
    file << node->check_leaf << " ";
    
    // Salvar os valores do nó
    file << node->values.size() << " ";
    for (int val : node->values) {
        file << val << " ";
    }
    
    // Salvar os filhos do nó se não for folha
    if (!node->check_leaf) {
        file << node->children.size() << " ";
        for (Node* child : node->children) {
            saveTree(file, child);
        }
    }
}

void BplusTree::saveToFile(const std::string& filename) {
    std::ofstream file(filename, std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para salvar a árvore.\n";
        return;
    }
    saveTree(file, root);
    file.close();
}

void BplusTree::loadTree(std::ifstream& file, Node*& node, int order) {
    bool isLeaf;
    int numValues, numChildren;

    // Ler se o nó é folha
    file >> isLeaf;
    node = new Node(order);
    node->check_leaf = isLeaf;

    // Ler os valores do nó
    file >> numValues;
    for (int i = 0; i < numValues; ++i) {
        int val;
        file >> val;
        node->values.push_back(val);
    }

    // Se não é folha, ler os filhos
    if (!isLeaf) {
        file >> numChildren;
        for (int i = 0; i < numChildren; ++i) {
            Node* child = nullptr;
            loadTree(file, child, order);
            child->parent = node;
            node->children.push_back(child);
        }
    }
}

void BplusTree::loadFromFile(const std::string& filename, int order) {
    std::ifstream file(filename, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para carregar a árvore.\n";
        return;
    }
    root = nullptr;
    loadTree(file, root, order);
    file.close();
}


/*int main() {
    int order = 3;  // Ordem 3, conforme pedido
    BplusTree tree(order);
    std::string filename = "btree_data.txt";

    // Inserindo alguns valores
    for (int i = 1; i <= 10; ++i) {
        std::cout << "Inserindo valor: " << i << std::endl;
        tree.insert(i);
    }

    // Exibindo a estrutura da árvore
    std::cout << "Estrutura da B+ Tree (Ordem " << order << "):" << std::endl;
    tree.display();

    // Salvando a árvore em um arquivo
    std::cout << "\nSalvando a árvore em arquivo..." << std::endl;
    tree.saveToFile(filename);

    // Carregando a árvore de um arquivo
    std::cout << "\nCarregando a árvore do arquivo..." << std::endl;
    BplusTree loadedTree(order);
    loadedTree.loadFromFile(filename, order);

    // Exibindo a estrutura da árvore carregada para verificar consistência
    std::cout << "Estrutura da B+ Tree carregada do arquivo:" << std::endl;
    loadedTree.display();

    return 0;
}*/

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
    //bplustree.printTree(bplustree.root);
    //cout << "\n" << endl;
    //cout << bplustree2.root->children[1]->values[0] << endl;

    std::cout << "Estrutura da B+Tree (Ordem 3): " << std::endl;
    bplustree.printTree(bplustree.root);

    return 0;
}
