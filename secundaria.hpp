#include <string>
#include <map>
#include <vector>

// Alterando o tipo de chave de int para std::string para índice secundário
class NodeSec {
public:
    int order;
    bool check_leaf;
    std::vector<std::string> values;  // Mudando de int para std::string para suportar títulos
    std::vector<std::vector<std::streampos>> keys;  // Armazena múltiplos endereços para uma mesma chave
    std::vector<NodeSec*> children;
    NodeSec* nextKey;
    NodeSec* parent;

    NodeSec(int order);
    void insert_at_leaf(const std::string& title, std::streampos key);  // Alteração no tipo da chave
};

class BplusTreeSec {
public:
    NodeSec* root;
    int order;

    BplusTreeSec(int order);
    NodeSec* search(const std::string& title);  // Alteração no tipo da chave
    void insert(const std::string& title, std::streampos key);  // Alteração no tipo da chave
    bool find(const std::string& title, std::streampos key);  // Alteração no tipo da chave
    std::streampos searchKey(const std::string value);  // Retorna todos os endereços associados
    void insert_in_parent(NodeSec* nodeSec, const std::string& title, NodeSec* newLeaf);
    void split(NodeSec* parentNodeSec);
    void printLeaves();
    void printTree(NodeSec* nodeSec, int level = 0);

    // Funções de salvar e carregar a árvore
    void saveTree(std::ofstream& file, NodeSec* nodeSec);
    void saveToFile(const std::string& filename);
    void loadTree(std::ifstream& file, NodeSec*& nodeSec, int order);
    void loadFromFile(const std::string& filename);
};
