#include <vector>
#include <iostream>
#include <memory>

template <typename Key>
class BPlusTree {
private:
    struct Nozin {
        bool isLeaf;
        std::vector<Key> keys;
        void** ponteiros; // Mudança para ponteiros genéricos
        Nozin* pae;
        int numKeys;

        Nozin(bool leaf) : isLeaf(leaf), numKeys(0) {
            keys.resize(2); // Capacidade inicial
            ponteiros = new void*[3]; // Capacidade inicial
        }

        ~Nozin() {
            delete[] ponteiros; // Liberar a memória alocada para ponteiros
        }
    };

    Nozin* root;

public:
    struct Block { // Tornar público
        int endereco;
    };

    BPlusTree() : root(nullptr) {}

    void insert(Key key, Block* bloco);
    void display(Nozin* no, int level = 0);
    Nozin* getRoot() { return root; }
    void insertNonFull(Nozin* no, Key key, Block* bloco);
    void splitChild(Nozin* parent, int index, Nozin* child);
};

template <typename Key>
void BPlusTree<Key>::insert(Key key, Block* bloco) {
    if (root == nullptr) {
        // Criar a raiz se não existir
        root = new Nozin(true);
        root->keys[0] = key;
        root->ponteiros[0] = bloco; // Usar ponteiros para armazenar o bloco
        root->numKeys = 1;
    } else {
        if (root->numKeys == 2) { // Se a raiz está cheia
            Nozin* newRoot = new Nozin(false);
            newRoot->ponteiros[0] = root;
            splitChild(newRoot, 0, root);
            root = newRoot;
            insertNonFull(root, key, bloco);
        } else {
            insertNonFull(root, key, bloco);
        }
    }
}

template <typename Key>
void BPlusTree<Key>::insertNonFull(Nozin* no, Key key, Block* bloco) {
    int i = no->numKeys - 1;

    if (no->isLeaf) {
        while (i >= 0 && key < no->keys[i]) {
            no->keys[i + 1] = no->keys[i];
            no->ponteiros[i + 1] = no->ponteiros[i]; // Mover ponteiros
            i--;
        }
        no->keys[i + 1] = key;
        no->ponteiros[i + 1] = bloco; // Armazenar o bloco
        no->numKeys++;
    } else {
        while (i >= 0 && key < no->keys[i]) {
            i--;
        }
        i++;
        if (static_cast<Nozin*>(no->ponteiros[i])->numKeys == 2) { // Se o nó filho está cheio
            splitChild(no, i, static_cast<Nozin*>(no->ponteiros[i]));
            if (key > no->keys[i]) {
                i++;
            }
        }
        insertNonFull(static_cast<Nozin*>(no->ponteiros[i]), key, bloco);
    }
}

template <typename Key>
void BPlusTree<Key>::splitChild(Nozin* parent, int index, Nozin* child) {
    Nozin* newChild = new Nozin(child->isLeaf);
    newChild->numKeys = 1; // Inicializa com 1 chave

    // Move a metade das chaves e ponteiros do filho para o novo filho
    newChild->keys[0] = child->keys[1];

    if (!child->isLeaf) {
        newChild->ponteiros[0] = child->ponteiros[1];
    }

    child->numKeys = 1; // Atualiza o número de chaves no filho original
    parent->keys.insert(parent->keys.begin() + index, child->keys[0]); // Insere a nova chave na raiz
    parent->ponteiros = static_cast<void**>(realloc(parent->ponteiros, (parent->numKeys + 1) * sizeof(void*))); // Re-alocar ponteiros
    parent->ponteiros[index + 1] = newChild; // Insere o novo filho
    parent->numKeys++;
}

template <typename Key>
void BPlusTree<Key>::display(Nozin* no, int level) {
    if (no == nullptr) return;

    std::cout << "Level " << level << ": ";
    for (int i = 0; i < no->numKeys; i++) {
        std::cout << no->keys[i] << " ";
    }
    std::cout << std::endl;

    if (!no->isLeaf) {
        for (int i = 0; i <= no->numKeys; i++) {
            display(static_cast<Nozin*>(no->ponteiros[i]), level + 1);
        }
    }
}

int main() {
    BPlusTree<int> bPlusTree;

    // Inserindo alguns dados
    for (int i = 1; i <= 5; i++) {
        BPlusTree<int>::Block* bloco = new BPlusTree<int>::Block();
        bloco->endereco = i * 10; // Exemplo de endereço
        bPlusTree.insert(i, bloco);
    }

    // Exibindo a árvore
    std::cout << "B+ Tree structure:" << std::endl;
    bPlusTree.display(bPlusTree.getRoot());

    return 0;
}
