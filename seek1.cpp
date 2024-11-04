#include <iostream>
#include "btree.h"
#include "HashUpload.h"
#include <cstdint>

Article seek1(BplusTree bplustree, void* endereco, const std::string& bucket_filename, const std::string& overflow_filename) {
    Article article;

    // Abrir o arquivo de bucket
    std::ifstream bucket_file(bucket_filename, std::ios::binary);
    if (!bucket_file) {
        std::cerr << "Erro ao abrir o arquivo de bucket: " << bucket_filename << std::endl;
        return article; // Retorna um artigo vazio em caso de erro
    }

    // Ler o bloco correspondente ao endereço
    bucket_file.seekg(static_cast<std::streamoff>(reinterpret_cast<std::uintptr_t>(endereco)), std::ios::beg);
    bucket_file.read(reinterpret_cast<char*>(&article), sizeof(Article));

    bucket_file.close();

    // Verifique se o artigo foi lido corretamente (opcional)
    if (article.id == 0) { // Supondo que 0 é um ID inválido
        std::cerr << "Artigo não encontrado ou ID inválido." << std::endl;
        // Trate o caso de erro se necessário
    }

    return article;
}

int main(int argc, char const *argv[])
{
    int order = 3;
    BplusTree bplustree2(order);

    bplustree2.loadFromFile("index.bin", order);
    //bplustree2.printTree(bplustree2.root);
    bplustree2.printTree(bplustree2.root);
    void* endereco = bplustree2.search(3)->keys[0][0];


    
}
