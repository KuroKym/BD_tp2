#include "secundaria.hpp"
#include <fstream>
#include <iostream>
using namespace std;

NodeSec::NodeSec(int order) : order(order), check_leaf(true), nextKey(nullptr), parent(nullptr) {}

void NodeSec::insert_at_leaf(const std::string& title, std::streampos key) {
    if (!values.empty()) {
        for (int i = 0; i < values.size(); i++) {
            if (title == values[i]) {
                // Título já existe na árvore
                keys[i].push_back(key);
                return;
            } else if (title < values[i]) {
                values.insert(values.begin() + i, title);
                keys.insert(keys.begin() + i, std::vector<std::streampos>{key});
                return;
            }
        }
    }
    values.push_back(title);
    keys.push_back(std::vector<std::streampos>{key});
}


BplusTreeSec::BplusTreeSec(int order) : order(order) {
    root = new NodeSec(order);
}

NodeSec* BplusTreeSec::search(const std::string& title) {
    NodeSec* currentNodeSec = root;
    while (!currentNodeSec->check_leaf) {
        for (int i = 0; i < currentNodeSec->values.size(); i++) {
            if (title < currentNodeSec->values[i]) {
                currentNodeSec = currentNodeSec->children[i];
                break;
            } else if (i + 1 == currentNodeSec->values.size()) {
                currentNodeSec = currentNodeSec->children[i + 1];
                break;
            }
        }
    }
    return currentNodeSec;
}

void BplusTreeSec::insert(const std::string& title, std::streampos key) {
    NodeSec* leafNodeSec = search(title);
    leafNodeSec->insert_at_leaf(title, key);

    if (leafNodeSec->values.size() == order) {
        NodeSec* newLeaf = new NodeSec(order);
        newLeaf->check_leaf = true;
        newLeaf->parent = leafNodeSec->parent;

        int mid = (order + 1) / 2;
        newLeaf->values.assign(leafNodeSec->values.begin() + mid, leafNodeSec->values.end());
        newLeaf->keys.assign(leafNodeSec->keys.begin() + mid, leafNodeSec->keys.end());
        newLeaf->nextKey = leafNodeSec->nextKey;

        leafNodeSec->values.resize(mid);
        leafNodeSec->keys.resize(mid);
        leafNodeSec->nextKey = newLeaf;

        insert_in_parent(leafNodeSec, newLeaf->values[0], newLeaf);
    }
}


void BplusTreeSec::insert_in_parent(NodeSec* nodeseNodeSec, const std::string& value, NodeSec* newLeaf) {
    if (root == nodeseNodeSec) {
        NodeSec* newRoot = new NodeSec(order);
        newRoot->values.push_back(value);
        newRoot->children.push_back(nodeseNodeSec);
        newRoot->children.push_back(newLeaf);
        newRoot->check_leaf = false;
        root = newRoot;
        nodeseNodeSec->parent = newRoot;
        newLeaf->parent = newRoot;
        return;
    }

    NodeSec* parentNodeSec = nodeseNodeSec->parent;
    int insertPos = 0;
    while (insertPos < parentNodeSec->values.size() && value > parentNodeSec->values[insertPos]) {
        insertPos++;
    }

    parentNodeSec->values.insert(parentNodeSec->values.begin() + insertPos, value);
    parentNodeSec->children.insert(parentNodeSec->children.begin() + insertPos + 1, newLeaf);
    newLeaf->parent = parentNodeSec;

    if (parentNodeSec->values.size() == order) {
        split(parentNodeSec);
    }
}

void BplusTreeSec::split(NodeSec* parentNodeSec) {
    NodeSec* newInternal = new NodeSec(order);
    newInternal->check_leaf = false;
    newInternal->parent = parentNodeSec->parent;

    int mid = order / 2;
    std::string midValue = parentNodeSec->values[mid];

    newInternal->values.assign(parentNodeSec->values.begin() + mid + 1, parentNodeSec->values.end());
    newInternal->children.assign(parentNodeSec->children.begin() + mid + 1, parentNodeSec->children.end());

    for (NodeSec* child : newInternal->children) {
        child->parent = newInternal;
    }

    parentNodeSec->values.resize(mid);
    parentNodeSec->children.resize(mid + 1);

    if (parentNodeSec == root) {
        NodeSec* newRoot = new NodeSec(order);
        newRoot->check_leaf = false;
        newRoot->values.push_back(midValue);
        newRoot->children.push_back(parentNodeSec);
        newRoot->children.push_back(newInternal);
        root = newRoot;
        parentNodeSec->parent = newRoot;
        newInternal->parent = newRoot;
    } else {
        insert_in_parent(parentNodeSec, midValue, newInternal);
    }
}

void BplusTreeSec::printTree(NodeSec* nodeseNodeSec, int level) {
    if (!nodeseNodeSec) return;

    // std::cout << "Nível " << level << ": ";
    for (const auto& value : nodeseNodeSec->values) {
        // std::cout << value << " ";
    }
    // std::cout << std::endl;

    if (!nodeseNodeSec->check_leaf) {
        for (NodeSec* child : nodeseNodeSec->children) {
            printTree(child, level + 1);
        }
    }
}

void BplusTreeSec::printLeaves() {
    NodeSec* current = root;
    while (current && !current->check_leaf) {
        current = current->children[0];
    }

    while (current) {
        for (const auto& value : current->values) {
            // std::cout << value << " ";
        }
        // std::cout << " -> ";
        current = current->nextKey;
    }
    // std::cout << "nullptr" << std::endl;
}

// Função para salvar a árvore em um arquivo
void BplusTreeSec::saveTree(ofstream& file, NodeSec* nodeseNodeSec) {
    if (!nodeseNodeSec) return;

    // Salva informações do nó
    file.write((char*)&nodeseNodeSec->check_leaf, sizeof(nodeseNodeSec->check_leaf));
    int numValues = nodeseNodeSec->values.size();
    file.write((char*)&numValues, sizeof(numValues));
    // cout << "Salvando nó: " << (nodeseNodeSec->check_leaf ? "Folha" : "Interno") << " com " << numValues << " valores." << endl;

    for (const std::string& value : nodeseNodeSec->values) {
        int valueSize = value.size();
        file.write((char*)&valueSize, sizeof(valueSize)); // Salva o tamanho da string
        file.write(value.c_str(), valueSize); // Salva o conteúdo da string
        // std::cout << "Salvando valor: " << value << std::endl;
    }


    int numKeys = nodeseNodeSec->keys.size();
    file.write((char*)&numKeys, sizeof(numKeys));
    for (const auto& keyList : nodeseNodeSec->keys) {
        int keyListSize = keyList.size();
        file.write((char*)&keyListSize, sizeof(keyListSize));
        for (std::streampos key : keyList) {
            std::streamoff keyAddr = key;
            file.write((char*)&keyAddr, sizeof(keyAddr));
            // cout << "Salvando chave: " << keyAddr << endl;
        }
    }

    if (nodeseNodeSec->check_leaf) {
        long nextKeyAddr = (nodeseNodeSec->nextKey) ? reinterpret_cast<long>(nodeseNodeSec->nextKey) : -1;
        file.write((char*)&nextKeyAddr, sizeof(nextKeyAddr));
        // cout << "Salvando endereço do próximo nó chave: " << nextKeyAddr << endl;
    } else {
        int numChildren = nodeseNodeSec->children.size();
        file.write((char*)&numChildren, sizeof(numChildren));
        // cout << "Salvando número de filhos: " << numChildren << endl;
        for (NodeSec* child : nodeseNodeSec->children) {
            long childAddr = reinterpret_cast<long>(child);
            file.write((char*)&childAddr, sizeof(childAddr));
            // cout << "Salvando endereço do filho: " << childAddr << endl;
        }
    }

    // Salvar recursivamente os filhos
    for (NodeSec* child : nodeseNodeSec->children) {
        saveTree(file, child);
    }
}

// Função para salvar a árvore em um arquivo
void BplusTreeSec::saveToFile(const string& filename) {
    ofstream file(filename, ios::binary);
    if (file.is_open()) {
        saveTree(file, root);
        file.close();
        //  cout << "Árvore salva com sucesso em " << filename << endl;
    } else {
        //  cout << "Erro ao abrir o arquivo." << endl;
    }
}

// Função para carregar a árvore de um arquivo
void BplusTreeSec::loadTree(ifstream& file, NodeSec*& nodeseNodeSec, int order) {
    if (!file.is_open() || file.eof()) {
        // std::cout << "Arquivo não aberto ou EOF atingido. Encerrando carregamento." << std::endl;
        return;
    }

    // Ler informações do nó
    bool isLeaf;
    file.read((char*)&isLeaf, sizeof(isLeaf));
    // std::cout << "Carregando nó: " << (isLeaf ? "Folha" : "Interno") << std::endl;

    nodeseNodeSec = new NodeSec(order);
    nodeseNodeSec->check_leaf = isLeaf;

    int numValues;
    file.read((char*)&numValues, sizeof(numValues));
    nodeseNodeSec->values.resize(numValues);
    // std::cout << "Número de valores no nó: " << numValues << std::endl;

    for (int i = 0; i < numValues; ++i) {
        int valueSize;
        file.read((char*)&valueSize, sizeof(valueSize)); // Lê o tamanho da string

        std::string value(valueSize, '\0'); // Cria uma string de tamanho `valueSize`
        file.read(&value[0], valueSize); // Lê o conteúdo da string
        nodeseNodeSec->values[i] = value; // Armazena o valor lido no nó

        // std::cout << "Valor[" << i << "]: " << nodeseNodeSec->values[i] << std::endl;
    }


    int numKeys;
    file.read((char*)&numKeys, sizeof(numKeys));
    nodeseNodeSec->keys.resize(numKeys);
    for (int i = 0; i < numKeys; ++i) {
        int keyListSize;
        file.read((char*)&keyListSize, sizeof(keyListSize));
        nodeseNodeSec->keys[i].resize(keyListSize);
        for (int j = 0; j < keyListSize; ++j) {
            long keyAddr;
            file.read((char*)&keyAddr, sizeof(keyAddr));
            nodeseNodeSec->keys[i][j] = static_cast<std::streampos>(keyAddr);
            // cout << "Chave[" << i << "][" << j << "]: " << keyAddr << endl;
        }
    }

    if (isLeaf) {
        long nextKeyAddr;
        file.read((char*)&nextKeyAddr, sizeof(nextKeyAddr));
        nodeseNodeSec->nextKey = reinterpret_cast<NodeSec*>(nextKeyAddr);
        // std::cout << "Endereço do próximo nó chave: " << nextKeyAddr << std::endl;
    } else {
        int numChildren;
        file.read((char*)&numChildren, sizeof(numChildren));
        nodeseNodeSec->children.resize(numChildren);
        // std::cout << "Número de filhos: " << numChildren << std::endl;

        for (int i = 0; i < numChildren; ++i) {
            long childAddr;
            file.read((char*)&childAddr, sizeof(childAddr));
            nodeseNodeSec->children[i] = reinterpret_cast<NodeSec*>(childAddr);
            // std::cout << "Endereço do filho[" << i << "]: " << childAddr << std::endl;
        }
    }

    // Carregar recursivamente os filhos
    for (int i = 0; i < nodeseNodeSec->children.size(); ++i) {
        loadTree(file, nodeseNodeSec->children[i], order);
        nodeseNodeSec->children[i]->parent = nodeseNodeSec; // Correctly link the parent
    }
}


// Função para carregar a árvore de um arquivo
void BplusTreeSec::loadFromFile(const std::string& filename) {
    ifstream file(filename, ios::binary);
    if (file.is_open()) {
        loadTree(file, root, order);
        file.close();
        // cout << "Árvore carregada com sucesso de " << filename << endl;
    } else {
        // cout << "Erro ao abrir o arquivo." << endl;
    }
}



// int main() {
//     int order = 3;
//     BplusTreeSec bplustreeSec(order);
//     BplusTree bplustree2(order);

//     int id = 1;
//     void* blockPtr = reinterpret_cast<void*>(0x1000);
//     bplustree.insert(id, blockPtr);
//     id = 2;
//     blockPtr = reinterpret_cast<void*>(0x1200);
//     bplustree.insert(id, blockPtr);
//     id = 3;
//     blockPtr = reinterpret_cast<void*>(0x1400); // Example block pointer
//     bplustree.insert(id, blockPtr);
//     id = 4;
//     blockPtr = reinterpret_cast<void*>(0x1600); // Example block pointer
//     bplustree.insert(id, blockPtr);
//     id = 5;
//     blockPtr = reinterpret_cast<void*>(0x1800); // Example block pointer
//     bplustree.insert(id, blockPtr);
//     id = 6;
//     blockPtr = reinterpret_cast<void*>(0x2000); // Example block pointer
//     bplustree.insert(id, blockPtr);
//     id = 7;
//     blockPtr = reinterpret_cast<void*>(0x2200); // Example block pointer
//     bplustree.insert(id, blockPtr);
//     id = 8;
//     blockPtr = reinterpret_cast<void*>(0x2400); // Example block pointer
//     bplustree.insert(id, blockPtr);
//     id = 9;
//     blockPtr = reinterpret_cast<void*>(0x2600); // Example block pointer
//     bplustree.insert(id, blockPtr);
//     id = 10;
//     blockPtr = reinterpret_cast<void*>(0x2800); // Example block pointer
//     bplustree.insert(id, blockPtr);

//     bplustree.saveToFile("index.bin");
//     bplustree2.loadFromFile("index.bin", order);
//     //bplustree.printTree(bplustree.root);
//     //cout << "\n" << endl;
//     //cout << bplustree2.root->children[1]->values[0] << endl;

//     // std::cout << "Estrutura da B+Tree (Ordem 3): " << std::endl;
//     // bplustree2.printTree(bplustree2.root);
//     bplustree2.printLeaves();
//     cout << endl;
//     cout << "começando a busca" << endl;
//     cout << "Buscando chave do id 3: " << bplustree.search(3)->keys[0][0]<< endl;
//     cout << "Buscando chave do id 3: " << bplustree2.search(3)->keys[0][0]<< endl;


//     return 0;
// }

std::streampos BplusTreeSec::searchKey(const std::string value) {
    std::cout << "Iniciando busca pela chave: " << value << std::endl;
    NodeSec* currentNodeSec = search(value);

    // Remove as aspas de `value` para comparação
    std::string cleanedValue = value;
    if (cleanedValue.front() == '"' && cleanedValue.back() == '"') {
        cleanedValue = cleanedValue.substr(1, cleanedValue.size() - 2);
    }
    std::cout << "Valor limpo para busca: " << cleanedValue << std::endl;

    for (int i = 0; i < currentNodeSec->values.size(); i++) {
        // Remove as aspas de `currentNodeSec->values[i]`
        std::string cleanedCurrentValue = currentNodeSec->values[i];
        if (cleanedCurrentValue.front() == '"' && cleanedCurrentValue.back() == '"') {
            cleanedCurrentValue = cleanedCurrentValue.substr(1, cleanedCurrentValue.size() - 2);
        }
        std::cout << "Comparando com valor no nó: " << cleanedCurrentValue << std::endl;

        if (cleanedValue == cleanedCurrentValue) {
            std::cout << "Chave encontrada: " << currentNodeSec->keys[i][0] << std::endl;
            return currentNodeSec->keys[i][0];
        }
    }

    std::cout << "Chave não encontrada." << std::endl;
    return -1;
}



// int main(){
//     // BplusTreeSec bptree(3);
//     BplusTreeSec bpCarregada(3);
//     // bptree.insert("Poster: Portable integral photography input/ output system using tablet PC and fly's eye lenses.", 1230);
//     // bptree.saveToFile("secIndex.bin");
//     bpCarregada.loadFromFile("indexSec.bin");
//     // bptree.loadFromFile("indexSec.bin");
//     // bpCarregada.printTree(bpCarregada.root);
//     // cout << "endereco: "<< bptree.searchKey("Poster: Portable integral photography input/ output system using tablet PC and fly's eye lenses.") << endl;
//     // cout << "endereco: "<< bpCarregada.root->values[0] << endl;
//     cout << "titulo: " << bpCarregada.search("MakeVR: A 3D world-building interface.")->values[0] << endl;
//     return 0;
// }