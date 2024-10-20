#include "HashUpload.h"
#include "btree.cpp"


Article* buscarArtigoPeloIndice(int id, const std::string& index_filename) {
    BPlusTree<int, Article> indexTree(4);
    indexTree.load(index_filename); // Carrega a árvore do arquivo

    Article* artigo = indexTree.search(id); // Busca pelo ID
    if (artigo) {
        return artigo; // Retorna o artigo encontrado
    } else {
        std::cerr << "Artigo não encontrado." << std::endl;
        return nullptr; // Retorna nulo se não encontrar
    }
}


int main(int argc, char const *argv[])
{
    Article* artigo = nullptr;
    int id_to_search;
    std::cout << "Digite o ID do artigo a ser buscado: ";
    std::cin >> id_to_search;
    artigo = buscarArtigoPeloIndice(id_to_search, "index.bin");
    if (artigo) {
        // Imprime as informações do artigo encontrado
        std::cout << "Registro encontrado:" << std::endl;
        std::cout << "ID: " << artigo->id << std::endl;
        std::cout << "Título: " << artigo->title << std::endl;
        std::cout << "Ano: " << artigo->year << std::endl;
        std::cout << "Autores: " << artigo->authors << std::endl;
        std::cout << "Citações: " << artigo->citations << std::endl;
        std::cout << "Data de Atualização: " << artigo->updated_at << std::endl;
        std::cout << "Snippet: " << artigo->snippet << std::endl;
    }
    return 0;
}
